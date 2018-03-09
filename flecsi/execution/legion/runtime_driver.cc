/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
/*! @file */

#include <flecsi/execution/legion/runtime_driver.h>

#include <legion.h>
#include <legion_utilities.h>
#include <limits>

#include <flecsi/data/legion/legion_data.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/execution/legion/mapper.h>
#include <flecsi/execution/legion/internal_field.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>
#include <flecsi/data/data_constants.h> 

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
runtime_driver(
  const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime
)
{
  using namespace data;
  //using data::storage_label_type_t;

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "In Legion runtime driver" << std::endl;
  }

  // Get the input arguments from the Legion runtime
  const Legion::InputArgs & args =
    Legion::Runtime::get_input_args();

  // Initialize MPI Interoperability
  context_t & context_ = context_t::instance();
  context_.connect_with_mpi(ctx, runtime);
  context_.wait_on_mpi(ctx, runtime);

  using field_info_t = context_t::field_info_t;

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the client registry.
  //
  // NOTE: This needs to be called before the field registry below because
  //       The client callbacks register field callbacks with the field
  //       registry.
  //--------------------------------------------------------------------------//

  auto & client_registry =
    flecsi::data::storage_t::instance().client_registry();

  for(auto & c: client_registry) {
    for(auto & d: c.second) {
      d.second.second(d.second.first);
    } // for
  } // for

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the field registry.
  //--------------------------------------------------------------------------//

  auto & field_registry =
    flecsi::data::storage_t::instance().field_registry();

  for(auto & c: field_registry) {
    for(auto & f: c.second) {
      f.second.second(f.first, f.second.first);
    } // for
  } // for

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI num_colors is " << num_colors << std::endl;
  }

  data::legion_data_t data(ctx, runtime, num_colors);
  
  data.init_global_handles();

  size_t number_of_global_fields = 0;
  for(const field_info_t& field_info : context_.registered_fields()){
    context_.put_field_info(field_info);
    if (field_info.storage_class == global)
      number_of_global_fields++;
  }

  if (number_of_global_fields > 0)
  {
    auto& ispace_dmap = context_.index_space_data_map();
    size_t global_index_space =
      execution::internal_index_space::global_is;

    ispace_dmap[global_index_space].entire_region =
        data.global_index_space().logical_region;
  }

#if defined FLECSI_ENABLE_SPECIALIZATION_TLT_INIT
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization top-level-task init" << std::endl;
  }

  // Invoke the specialization top-level task initialization function.
  specialization_tlt_init(args.argc, args.argv);

#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT


  //--------------------------------------------------------------------------//
  //  Create Legion index spaces and logical regions
  //-------------------------------------------------------------------------//

  auto coloring_info = context_.coloring_info_map();

  data.init_from_coloring_info_map(coloring_info);

  for(auto& itr : context_.adjacency_info()){
    data.add_adjacency(itr.second);
  }

  data.finalize(coloring_info);

  //-------------------------------------------------------------------------//
  //  Create Legion reduction 
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Excute Legion task to maps between pre-compacted and compacted
  // data placement
  //-------------------------------------------------------------------------//
  
  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);
 
  const auto pos_compaction_id =
    context_.task_id<__flecsi_internal_task_key(owner_pos_compaction_task)>();
  
  Legion::IndexLauncher pos_compaction_launcher(pos_compaction_id,
      data.color_domain(), Legion::TaskArgument(nullptr, 0),
      Legion::ArgumentMap());
  
  pos_compaction_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition color_lpart = runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.color_partition);
        pos_compaction_launcher.add_region_requirement(
        Legion::RegionRequirement(color_lpart, 0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);
  } // for idx_space

  Legion::MustEpochLauncher must_epoch_launcher1;
  must_epoch_launcher1.launch_domain = data.color_domain();
  must_epoch_launcher1.add_index_task(pos_compaction_launcher);
  runtime->execute_must_epoch(ctx, must_epoch_launcher1);

  // Fix ghost reference/pointer to point to compacted position of
  // shared that it needs
  const auto pos_correction_id =
    context_.task_id<__flecsi_internal_task_key(owner_pos_correction_task)>();

  Legion::IndexLauncher fix_ghost_refs_launcher(pos_correction_id,
    data.color_domain(), Legion::TaskArgument(nullptr, 0), Legion::ArgumentMap());

  fix_ghost_refs_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition ghost_lpart = runtime->get_logical_partition(ctx,
    flecsi_ispace.logical_region, flecsi_ispace.ghost_partition);
    fix_ghost_refs_launcher.add_region_requirement(
    Legion::RegionRequirement(ghost_lpart, 0/*projection ID*/,
      READ_WRITE, EXCLUSIVE, flecsi_ispace.logical_region))
      .add_field(ghost_owner_pos_fid);

    Legion::LogicalPartition access_lp =runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.access_partition);
    Legion::LogicalRegion primary_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, PRIMARY_ACCESS);

    fix_ghost_refs_launcher.add_region_requirement(
       Legion::RegionRequirement(primary_lr, READ_ONLY, EXCLUSIVE,
           flecsi_ispace.logical_region)
            .add_field(ghost_owner_pos_fid));
  } // for idx_space

  Legion::MustEpochLauncher must_epoch_launcher2;
  must_epoch_launcher2.launch_domain = data.color_domain();
  must_epoch_launcher2.add_index_task(fix_ghost_refs_launcher);
  auto fm = runtime->execute_must_epoch(ctx, must_epoch_launcher2);

  fm.wait_all_results(true);

  // map of index space to the field_ids that are mapped to this index space
  std::map<size_t, std::vector<field_id_t>> fields_map;

  size_t number_of_color_fields = 0;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
      switch(field_info.storage_class){
        case global:
          number_of_global_fields++;
          break;
        case color:
          number_of_color_fields++;
          break;
        case local:
          break;
        default:
          if(field_info.index_space == idx_space){
            fields_map[idx_space].push_back(field_info.fid);
          } // if
          break;
      }
    } // for

    {
      clog_tag_guard(runtime_driver);
      clog(trace) << "fields_map[" <<idx_space<<"] has "<<
        fields_map[idx_space].size()<< " fields"<<std::endl;
    } // scope
  } // for


  //--------------------------------------------------------------------------//
  //   Create Legion must epoch launcher and add Region requirements
  //-------------------------------------------------------------------------//
  
  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;

  std::map<size_t,Legion::Serializer> args_serializers;

  const auto setup_rank_context_id =
    context_.task_id<__flecsi_internal_task_key(setup_rank_context_task)>();

  // Add colors to must_epoch_launcher
  for(size_t color(0); color<num_colors; ++color) {
   
    //-----------------------------------------------------------------------//
    // data serialization:
    //-----------------------------------------------------------------------//
    
    // #2 serialize field info
    size_t num_fields = context_.registered_fields().size();
    args_serializers[color].serialize(&num_fields, sizeof(size_t));
    args_serializers[color].serialize(
      &context_.registered_fields()[0], num_fields * sizeof(field_info_t));

   //-----------------------------------------------------------------------//
   //add region requirements to the setup_rank_context_launcher
   //-----------------------------------------------------------------------//
   
    Legion::TaskLauncher setup_rank_context_launcher(setup_rank_context_id,
        Legion::TaskArgument(args_serializers[color].get_buffer(),
                             args_serializers[color].get_used_bytes()));
    setup_rank_context_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, setup_rank_context_launcher);
  } // for color

  // Launch the setup_rank_context tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  bool silence_warnings = true;
  future.wait_all_results(silence_warnings);

  // Add additional setup.
  //context_.advance_state();

  auto& ispace_dmap = context_.index_space_data_map();

  //fill ispace_dmap with logical partitions
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition access_lp =runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.access_partition);
    Legion::LogicalRegion primary_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, PRIMARY_ACCESS);
    Legion::LogicalRegion ghost_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, GHOST_ACCESS);

    ispace_dmap[idx_space].entire_region = flecsi_ispace.logical_region;
    ispace_dmap[idx_space].color_partition = runtime->get_logical_partition(
        ctx, flecsi_ispace.logical_region, flecsi_ispace.color_partition);
    runtime->attach_name(ispace_dmap[idx_space].color_partition, "color logical partition");
    ispace_dmap[idx_space].primary_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.primary_partition);
    runtime->attach_name(ispace_dmap[idx_space].primary_lp, "primary logical partition");
    ispace_dmap[idx_space].exclusive_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.exclusive_partition);
    runtime->attach_name(ispace_dmap[idx_space].exclusive_lp, "exclusive logical partition");
    ispace_dmap[idx_space].shared_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.shared_partition);
    runtime->attach_name(ispace_dmap[idx_space].shared_lp, "shared logical partition");
    ispace_dmap[idx_space].ghost_lp = runtime->get_logical_partition(
        ctx, ghost_lr, flecsi_ispace.ghost_partition);
    runtime->attach_name(ispace_dmap[idx_space].ghost_lp, "ghost logical partition");

    // Now that ghosts point to post-compacted shared positions, we can
    // partition set of shared positions that ghosts need
    Legion::IndexSpace is_of_colors = runtime->create_index_space(ctx,
        data.color_domain());

    ispace_dmap[idx_space].ghost_owners_ip = runtime->create_partition_by_image(ctx,
        primary_lr.get_index_space(), ispace_dmap[idx_space].ghost_lp,
        flecsi_ispace.logical_region, ghost_owner_pos_fid, is_of_colors);
    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_ip,
        "ghost owners index partition");
    ispace_dmap[idx_space].ghost_owners_lp = runtime->get_logical_partition(
        ctx, primary_lr, ispace_dmap[idx_space].ghost_owners_ip);
    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_lp,
        "ghost owners logical partition");
  } // idx_space

  // initialize read/write flags for task_prolog
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for (const field_id_t& field_id : fields_map[idx_space]){
      ispace_dmap[idx_space].ghost_is_readable[field_id] = true;
      ispace_dmap[idx_space].write_phase_started[field_id] = false;
    }//end field_info
  }//end for idx_space

  for (size_t idx : data.adjacencies()) {
    auto& adjacency = data.adjacency(idx);

    ispace_dmap[idx].entire_region = adjacency.logical_region;
    ispace_dmap[idx].color_partition =
        runtime->get_logical_partition(ctx,
            adjacency.logical_region, adjacency.index_partition);

  }

  //adding information for the global and color handles to the ispace_map

   if (number_of_global_fields>0){

    size_t global_index_space =
      execution::internal_index_space::global_is;

    auto global_ispace = data.global_index_space();

    ispace_dmap[global_index_space].entire_region = global_ispace.logical_region;
  }

  auto color_ispace = data.color_index_space();

  if(number_of_color_fields>0){
    size_t color_index_space =
      execution::internal_index_space::color_is;

    ispace_dmap[color_index_space].entire_region = color_ispace.logical_region;

    ispace_dmap[color_index_space].color_partition =
        runtime->get_logical_partition(ctx, color_ispace.logical_region,
            color_ispace.color_partition);
  }

  context_.advance_state();
  // run default or user-defined driver
  driver(args.argc, args.argv);

  //-----------------------------------------------------------------------//
  // Finish up Legion runtime and fall back out to MPI.
  // ----------------------------------------------------------------------//

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // runtime_driver

void
setup_rank_context_task(
  const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime
)
{
  using namespace data;

  const int my_color = task->index_point.point_data[0];

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing setup rank context task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  clog_assert(task->arglen > 0, "setup_rank_context_task called without arguments");

  //---------------------------------------------------------------------//
  // Deserialize task arguments
  // --------------------------------------------------------------------//

  Legion::Deserializer args_deserializer(task->args, task->arglen);
 
  // #2 deserialize field info
  size_t num_fields;
  args_deserializer.deserialize(&num_fields, sizeof(size_t));

  using field_info_t = context_t::field_info_t;
  auto field_info_buf = new field_info_t [num_fields];

  args_deserializer.deserialize(field_info_buf,
                                sizeof(field_info_t) * num_fields);

  // add field_info into the context (map between name, hash, is and field)
  if( (context_.field_info_map()).size()==0){
    for(size_t i = 0; i < num_fields; ++i){
      field_info_t& fi = field_info_buf[i];
      context_.put_field_info(fi);
    }//end for i
  }//if

  // Setup maps from mesh to compacted (local) index space and vice versa
  //
  // This depends on the ordering of the BLIS data structure setup.
  // Currently, this is Exclusive - Shared - Ghost.

  for(auto is: context_.coloring_map()) {
    std::map<size_t, size_t> _map;
    size_t counter(0);

    for(auto index: is.second.exclusive) {
      _map[counter++] = index.id;
    } // for

    for(auto index: is.second.shared) {
      _map[counter++] = index.id;
    } // for

    for(auto index: is.second.ghost) {
      _map[counter++] = index.id;
    } // for

    context_.add_index_map(is.first, _map);
  } // for coloring_map
  // FIXME: local_index_space is now problematic in control replication
  // It made sense in SPMD, but now each IndexLaunch starts from a clean slate.
#if 0
  auto& local_index_space_map = context_.local_index_space_map();
  for(auto& itr : local_index_space_map){
    size_t index_space = itr.first;

    legion_data_t::local_index_space_t lis = 
      legion_data_t::create_local_index_space(
      ctx, runtime, index_space, itr.second);
    
    auto& lism = context_.local_index_space_data_map();

    context_t::local_index_space_data_t lis_data;
    lis_data.region = lis.logical_region;
    lism.emplace(index_space, std::move(lis_data));
  }
#endif

} // setup_rank_context_task

} // namespace execution 
} // namespace flecsi
