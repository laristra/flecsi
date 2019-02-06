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
  }//scope

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI num_colors is " << num_colors << std::endl;
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

  data::legion_data_t data(ctx, runtime, num_colors);
  
  data.init_global_handles();

  size_t number_of_global_fields = 0;
  size_t number_of_sparse_fields = 0;

  for(const field_info_t& field_info : context_.registered_fields()){
    context_.put_field_info(field_info);
    switch(field_info.storage_class){
      case global:
        number_of_global_fields++;
        break;
      case ragged:
      case sparse:
        number_of_sparse_fields++;
        break;
      default:
        break;
    }
  }

  if (number_of_global_fields > 0)
  {
    auto& ispace_dmap = context_.index_space_data_map();
    size_t global_index_space =
      execution::internal_index_space::global_is;

    ispace_dmap[global_index_space].entire_region =
        data.global_index_space().logical_region;
  }//if

  if(number_of_sparse_fields > 0){
    data.init_sparse_metadata();
    auto& sparse_metadata = data.sparse_metadata();
    context_t::sparse_metadata_t md;
    md.entire_region = sparse_metadata.logical_region;
    md.color_partition = sparse_metadata.logical_partition;  
    context_.set_sparse_metadata(md);
  }//if

#if defined FLECSI_ENABLE_SPECIALIZATION_TLT_INIT
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization top-level-task init" << std::endl;
  }//scope

  // Invoke the specialization top-level task initialization function.
  specialization_tlt_init(args.argc, args.argv);

  context_.advance_state();
#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT


  //--------------------------------------------------------------------------//
  //  Create Legion index spaces and logical regions
  //-------------------------------------------------------------------------//

  auto coloring_info = context_.coloring_info_map();

  data.init_from_coloring_info_map(coloring_info,
    context_.sparse_index_space_info_map(), number_of_sparse_fields > 0);

  for(auto& itr : context_.adjacency_info()){
    data.add_adjacency(itr.second);
  }

  for(auto& itr : context_.index_subspace_info()){
    data.add_index_subspace(itr.second);
  }

  data.finalize(coloring_info);

  //-------------------------------------------------------------------------//
  // check nuber of fields allocated for sparse data
  //-------------------------------------------------------------------------//

  for(const field_info_t& field_info : context_.registered_fields()) {
    if(field_info.storage_class==sparse || field_info.storage_class==ragged ) {
        auto sparse_idx_space = field_info.index_space ;

        context_.increment_sparse_fields(sparse_idx_space);
    } // if
  } /// for 

  //-------------------------------------------------------------------------//
  // Excute Legion task to maps between pre-compacted and compacted
  // data placement
  //-------------------------------------------------------------------------//
  
  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);
 
  const auto pos_compaction_id =
    context_.task_id<flecsi_internal_task_key(owner_pos_compaction_task)>();
  
  Legion::IndexLauncher pos_compaction_launcher(pos_compaction_id,
      data.color_domain(), Legion::TaskArgument(nullptr, 0),
      Legion::ArgumentMap());
  
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

  pos_compaction_launcher.tag = MAPPER_FORCE_RANK_MATCH; 
  runtime->execute_index_space(ctx, pos_compaction_launcher);

  // Fix ghost reference/pointer to point to compacted position of
  // shared that it needs
  const auto pos_correction_id =
    context_.task_id<flecsi_internal_task_key(owner_pos_correction_task)>();

  Legion::IndexLauncher fix_ghost_refs_launcher(pos_correction_id,
    data.color_domain(), Legion::TaskArgument(nullptr, 0), Legion::ArgumentMap());

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

  fix_ghost_refs_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, fix_ghost_refs_launcher);
  fm.wait_all_results(true);

  // map of index space to the field_ids that are mapped to this index space
  std::map<size_t, std::vector<const field_info_t*>> fields_map;

  size_t number_of_color_fields = 0;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
      switch(field_info.storage_class){
        case global:
          //number_of_global_fields++;
          break;
        case color:
          number_of_color_fields++;
          break;
        case subspace:
        case local:
          break;
        default:
            fields_map[idx_space].push_back(&field_info);
          break;
      }
    } // for

  } // for


  //--------------------------------------------------------------------------//
  //   Create Legion must epoch launcher and add Region requirements
  //-------------------------------------------------------------------------//
  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;
  must_epoch_launcher.launch_domain = data.color_domain();

  std::map<size_t,Legion::Serializer> args_serializers;

  const auto setup_rank_context_id =
    context_.task_id<flecsi_internal_task_key(setup_rank_context_task)>();

   using sparse_index_space_info_t = context_t::sparse_index_space_info_t;

  // Add colors to must_epoch_launcher
  for(size_t color(0); color<num_colors; ++color) {

    size_t num_idx_spaces = context_.coloring_map().size();

    //-----------------------------------------------------------------------//
    // data serialization:
    //-----------------------------------------------------------------------//
    // #1a 
    args_serializers[color].serialize(&number_of_sparse_fields, sizeof(size_t));
    // #1 serialize sparse index spaces info

    std::vector<sparse_index_space_info_t> sparse_index_spaces_vec;
    
    for(auto& itr : context_.sparse_index_space_info_map()){
      sparse_index_spaces_vec.push_back(itr.second);
    }

    size_t num_sparse_index_spaces = sparse_index_spaces_vec.size();

    args_serializers[color].serialize(&num_sparse_index_spaces, 
      sizeof(size_t));
    
    args_serializers[color].serialize(&sparse_index_spaces_vec[0],
      num_sparse_index_spaces * sizeof(sparse_index_space_info_t));

    // #2 serialize field info
    size_t num_fields = context_.registered_fields().size();
    args_serializers[color].serialize(&num_fields, sizeof(size_t));
    args_serializers[color].serialize(
      &context_.registered_fields()[0], num_fields * sizeof(field_info_t));

    // #7 serialize adjacency info
    using adjacency_triple_t = context_t::adjacency_triple_t;

    std::vector<adjacency_triple_t> adjacencies_vec;

    for(auto& itr : context_.adjacency_info()){
      const coloring::adjacency_info_t& ai = itr.second;
      auto t = std::make_tuple(ai.index_space, ai.from_index_space,
        ai.to_index_space);
      adjacencies_vec.push_back(t);
    }//for

    size_t num_adjacencies = adjacencies_vec.size();

    args_serializers[color].serialize(&num_adjacencies, sizeof(size_t));
    args_serializers[color].serialize(&adjacencies_vec[0], num_adjacencies
      * sizeof(adjacency_triple_t));

    // #8 serialize index subspaces info

    using index_subspace_info_t = context_t::index_subspace_info_t;

    std::vector<index_subspace_info_t> index_subspaces_vec;

    for(auto& itr : context_.index_subspace_info()){
      const index_subspace_info_t& ii = itr.second;
      index_subspaces_vec.push_back(ii);
    }//for

    size_t num_index_subspaces = index_subspaces_vec.size();

    args_serializers[color].serialize(&num_index_subspaces, sizeof(size_t));
    args_serializers[color].serialize(&index_subspaces_vec[0],
      num_index_subspaces * sizeof(index_subspace_info_t));


   //-----------------------------------------------------------------------//
   // add setup_rank_context_launcher to must_epoch
   //-----------------------------------------------------------------------//
   
    Legion::TaskLauncher setup_rank_context_launcher(setup_rank_context_id,
        Legion::TaskArgument(args_serializers[color].get_buffer(),
                             args_serializers[color].get_used_bytes()));
    setup_rank_context_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    if(number_of_sparse_fields > 0){
      auto& sparse_metadata = data.sparse_metadata();
      Legion::LogicalRegion color_lregion =
        runtime->get_logical_subregion_by_color(ctx,
          sparse_metadata.logical_partition, color);

      Legion::RegionRequirement
        reg_req(color_lregion, READ_WRITE, SIMULTANEOUS,
          sparse_metadata.logical_region);

      for (const field_info_t & fi : context_.registered_fields()) {
        if (fi.storage_class == sparse || fi.storage_class == ragged) {
          reg_req.add_field(fi.fid);
        } // if
      } // for

      setup_rank_context_launcher.add_region_requirement(reg_req);

    }//end if sparse

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, setup_rank_context_launcher);
  } // for color

  // Launch the setup_rank_context tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  bool silence_warnings = true;
  future.wait_all_results(silence_warnings);


#if 0
  //launch a task that willl fill ghost_owner_pos_fid for the sparse
  //entity Logical Region
  const auto sparse_set_pos_id =
    context_.task_id<flecsi_internal_task_key(
    sparse_set_owner_position_task)>();
  Legion::IndexLauncher sparse_pos_launcher(sparse_set_pos_id,
    data.color_domain(), Legion::TaskArgument(nullptr, 0),
    Legion::ArgumentMap());

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    size_tsparse_idx_space = idx_space + 8192;
    auto& flecsi_is = data.index_space(idx_space);
    auto& flecsi_sis = data.index_space(sparse_idx_space);

    Legion::LogicalPartition color_lpart = runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.color_partition);
    Legion::LogicalPartition color_lpart = runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.color_partition);    

    sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(color_lpart, 0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);
  } // for idx_space

  pos_compaction_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  runtime->execute_index_space(ctx, pos_compaction_launcher);
#endif

  // Add additional setup.

  auto& ispace_dmap = context_.index_space_data_map();
  auto& sis_map = context_.sparse_index_space_info_map();

  Legion::Logger log_runtime("runtime");

  //fill ispace_dmap with logical partitions
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    size_t sparse_idx_space;

    const sparse_index_space_info_t* sparse_info;

    if(number_of_sparse_fields == 0){
      sparse_info = nullptr;
    }
    else{
      auto sitr = sis_map.find(idx_space);
      if(sitr != sis_map.end() &&
        (sitr->second.sparse_fields_registered_>0)){
        sparse_info = &sitr->second;
        // TODO: formalize sparse index space offset
        sparse_idx_space = idx_space + 8192;
      }
      else{
        sparse_info = nullptr;
      }//if
    }//if

    Legion::LogicalPartition access_lp =runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.access_partition);
    Legion::LogicalRegion primary_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, PRIMARY_ACCESS);
    Legion::LogicalRegion ghost_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, GHOST_ACCESS);

    ispace_dmap[idx_space].entire_region = flecsi_ispace.logical_region;


    ispace_dmap[idx_space].color_partition = runtime->get_logical_partition(
        ctx, flecsi_ispace.logical_region, flecsi_ispace.color_partition);
    runtime->attach_name(ispace_dmap[idx_space].color_partition,
			 "color logical partition");

    ispace_dmap[idx_space].primary_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.primary_partition);
    runtime->attach_name(ispace_dmap[idx_space].primary_lp,
			"primary logical partition");

    ispace_dmap[idx_space].exclusive_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.exclusive_partition);
    runtime->attach_name(ispace_dmap[idx_space].exclusive_lp,
			"exclusive logical partition");

    ispace_dmap[idx_space].shared_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.shared_partition);
    runtime->attach_name(ispace_dmap[idx_space].shared_lp,
			"shared logical partition");

    ispace_dmap[idx_space].ghost_lp = runtime->get_logical_partition(
        ctx, ghost_lr, flecsi_ispace.ghost_partition);
    runtime->attach_name(ispace_dmap[idx_space].ghost_lp,
			"ghost logical partition");

    // Now that ghosts point to post-compacted shared positions, we can
    // partition set of shared positions that ghosts need
    Legion::IndexSpace is_of_colors = runtime->create_index_space(ctx,
        data.color_domain());

    ispace_dmap[idx_space].ghost_owners_ip =
			runtime->create_partition_by_image(ctx,
        primary_lr.get_index_space(), ispace_dmap[idx_space].ghost_lp,
        flecsi_ispace.logical_region, ghost_owner_pos_fid, is_of_colors);
    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_ip,
        "ghost owners index partition");
    ispace_dmap[idx_space].ghost_owners_lp = runtime->get_logical_partition(
        ctx, primary_lr, ispace_dmap[idx_space].ghost_owners_ip);

    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_lp,
        "ghost owners logical partition");

    if(sparse_info){
     auto& flecsi_sis = data.sparse_index_space(idx_space);

      ispace_dmap[sparse_idx_space].entire_region = flecsi_sis.logical_region;

      Legion::LogicalPartition sis_access_lp =
				runtime->get_logical_partition(ctx,
        flecsi_sis.logical_region, flecsi_sis.access_partition);
			Legion::LogicalRegion sis_primary_lr =
				runtime->get_logical_subregion_by_color(
        ctx, sis_access_lp, PRIMARY_ACCESS);
			Legion::LogicalRegion sis_ghost_lr =
				runtime->get_logical_subregion_by_color(
        ctx, sis_access_lp, GHOST_ACCESS);

			ispace_dmap[sparse_idx_space].primary_lp = runtime->get_logical_partition(
        ctx, sis_primary_lr, flecsi_sis.primary_partition);
			runtime->attach_name(ispace_dmap[sparse_idx_space].primary_lp,
				"primary logical partition");

			ispace_dmap[sparse_idx_space].exclusive_lp =
				runtime->get_logical_partition(
        ctx, sis_primary_lr, flecsi_sis.exclusive_partition);
			runtime->attach_name(ispace_dmap[sparse_idx_space].exclusive_lp,
				"exclusive logical partition");

			ispace_dmap[sparse_idx_space].shared_lp = runtime->get_logical_partition(
        ctx, sis_primary_lr, flecsi_sis.shared_partition);
			runtime->attach_name(ispace_dmap[sparse_idx_space].shared_lp,
				"shared logical partition");

			ispace_dmap[sparse_idx_space].ghost_lp = runtime->get_logical_partition(
        ctx, sis_ghost_lr, flecsi_sis.ghost_partition);
			runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_lp,
				"ghost logical partition");
#if 0
      const auto sparse_set_pos_id =
        context_.task_id<flecsi_internal_task_key(
    		sparse_set_owner_position_task)>();
  		Legion::IndexLauncher sparse_pos_launcher(sparse_set_pos_id,
    		data.color_domain(), Legion::TaskArgument(nullptr, 0),
    		Legion::ArgumentMap());

      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_lp,
						0/*projection ID*/,
            READ_ONLY, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);

      Legion::FieldID fid;
      for ( const field_info_t & fi : context_.registered_fields()){
        if (fi.index_space == idx_space)
          if(!utils::hash::is_internal(fi.key))
            //if (sizeof(utils::offset_t) == fi.size)
              fid = fi.fid;
      }
     // fid =38;
      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_owners_lp,
            0/*projection ID*/,
            READ_ONLY, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(fid);
      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[sparse_idx_space].ghost_lp,
            0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_sis.logical_region))
                .add_field(ghost_owner_pos_fid);

      sparse_pos_launcher.tag = MAPPER_FORCE_RANK_MATCH;
      auto future = runtime->execute_index_space(ctx, sparse_pos_launcher); 
      future.wait_all_results(false);

      ispace_dmap[sparse_idx_space].ghost_owners_ip =
        runtime->create_partition_by_image(ctx,
        sis_primary_lr.get_index_space(),
				ispace_dmap[sparse_idx_space].ghost_lp,
        flecsi_sis.logical_region, ghost_owner_pos_fid, is_of_colors);

     runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_owners_ip,
        "ghost owners index partition");
     ispace_dmap[sparse_idx_space].ghost_owners_lp =
			runtime->get_logical_partition(
      ctx, sis_primary_lr, ispace_dmap[sparse_idx_space].ghost_owners_ip);
     runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_owners_lp,
        "ghost owners logical partition");

#endif

#if 0
			ispace_dmap[sparse_idx_space].ghost_owners_lp =
				runtime->get_logical_partition(
        ctx, flecsi_sis.logical_region, flecsi_sis.all_shared_partition);
#endif
//    runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_owners_lp,
//		"ghost owners logical partition");

    }
//  } // idx_space

  // initialize read/write flags for task_prolog
//  for(auto is: context_.coloring_map()) {
//    size_t idx_space = is.first;
    for (const field_info_t* field_info : fields_map[idx_space]){
      ispace_dmap[idx_space].ghost_is_readable[field_info->fid] = true;
      ispace_dmap[idx_space].write_phase_started[field_info->fid] = false;

      if(sparse_info){
        ispace_dmap[sparse_idx_space].ghost_is_readable[field_info->fid] = true;
        ispace_dmap[sparse_idx_space].write_phase_started[field_info->fid] = false;
      }
    }//end field_info
  }//end for idx_space
#if 0
 if (sparse)
    //launch a task that willl fill ghost_owner_pos_fid for the sparse
    //entity Logical Region
    const auto sparse_set_pos_id =
    context_.task_id<flecsi_internal_task_key(
    sparse_set_owner_position_task)>();
  Legion::IndexLauncher sparse_pos_launcher(sparse_set_pos_id,
    data.color_domain(), Legion::TaskArgument(nullptr, 0),
    Legion::ArgumentMap());

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    size_tsparse_idx_space = idx_space + 8192;
    auto& flecsi_is = data.index_space(idx_space);
    auto& flecsi_sis = data.index_space(sparse_idx_space);


    sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(color_lpart, 0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);
  } // for idx_space

  pos_compaction_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  runtime->execute_index_space(ctx, pos_compaction_launcher);
  }//endif
#endif 
 
  for (size_t idx : data.adjacencies()) {
    auto& adjacency = data.adjacency(idx);

    ispace_dmap[idx].entire_region = adjacency.logical_region;
    ispace_dmap[idx].color_partition =
        runtime->get_logical_partition(ctx,
            adjacency.logical_region, adjacency.index_partition);

  }

  // add subspace info to context

  auto& isubspace_dmap = context_.index_subspace_data_map();
  auto& data_subspace_map = data.index_subspace_map();

  for(auto& itr : context_.index_subspace_info()) {
    size_t index = itr.first;
    isubspace_dmap[index].logical_region =
        data_subspace_map.at(index).logical_region;
    isubspace_dmap[index].logical_partition =
        runtime->get_logical_partition(ctx,
            isubspace_dmap[index].logical_region, data_subspace_map.at(index).index_partition);
  }

  //adding information for the global and color handles to the ispace_map

   if (number_of_global_fields>0){

    size_t global_index_space =
      execution::internal_index_space::global_is;

    auto global_ispace = data.global_index_space();

    ispace_dmap[global_index_space].entire_region = global_ispace.logical_region;
  }//if

  auto color_ispace = data.color_index_space();

  if(number_of_color_fields>0){
    size_t color_index_space =
      execution::internal_index_space::color_is;

    ispace_dmap[color_index_space].entire_region = color_ispace.logical_region;

    ispace_dmap[color_index_space].color_partition =
        runtime->get_logical_partition(ctx, color_ispace.logical_region,
            color_ispace.color_partition);
  }//if

#if defined FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization spmd init" << std::endl;
  }//scope

  // Invoke the specialization top-level task initialization function.
   specialization_spmd_init(args.argc, args.argv);
  //
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

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

 
  auto& ispace_dmap = context_.index_space_data_map();
  auto& sis_map = context_.sparse_index_space_info_map();

  size_t region_index=0;
  //---------------------------------------------------------------------//
  // Deserialize task arguments
  // --------------------------------------------------------------------//

  Legion::Deserializer args_deserializer(task->args, task->arglen);
  //1a
  size_t number_of_sparse_fields;
  args_deserializer.deserialize(&number_of_sparse_fields, sizeof(size_t));

   // #1b deserialize sparse index spaces
  size_t num_sparse_index_spaces;
  args_deserializer.deserialize(&num_sparse_index_spaces, sizeof(size_t));

  using sparse_index_space_info_t = context_t::sparse_index_space_info_t;
  sparse_index_space_info_t* sparse_index_spaces =
     new sparse_index_space_info_t[num_sparse_index_spaces];

  args_deserializer.deserialize((void*)sparse_index_spaces,
    sizeof(sparse_index_space_info_t) * num_sparse_index_spaces);

  for(size_t i = 0; i < num_sparse_index_spaces; ++i){
    const sparse_index_space_info_t& si = sparse_index_spaces[i];
    context_.set_sparse_index_space_info(si);
  }


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
      context_.register_field_info(fi);
    }//end for i
  }//if

  // #7 deserialize adjacency info
  size_t num_adjacencies;
  args_deserializer.deserialize(&num_adjacencies, sizeof(size_t));

  using adjacency_triple_t = context_t::adjacency_triple_t;
  adjacency_triple_t* adjacencies =
     new adjacency_triple_t [num_adjacencies];

  args_deserializer.deserialize((void*)adjacencies,
    sizeof(adjacency_triple_t) * num_adjacencies);

  for(size_t i = 0; i < num_adjacencies; ++i){
    context_.add_adjacency_triple(adjacencies[i]);
  }

//  for(auto& itr : context_.adjacencies()) {
//    ispace_dmap[itr.first].color_region =
//      regions[region_index].get_logical_region();

//    region_index++;
//  }

  // #8 deserialize index subspaces
  size_t num_index_subspaces;
  args_deserializer.deserialize(&num_index_subspaces, sizeof(size_t));

  using index_subspace_info_t = context_t::index_subspace_info_t;
  index_subspace_info_t* index_subspaces =
     new index_subspace_info_t[num_index_subspaces];

  args_deserializer.deserialize((void*)index_subspaces,
    sizeof(index_subspace_info_t) * num_index_subspaces);

  for(size_t i = 0; i < num_index_subspaces; ++i){
    context_.add_index_subspace(index_subspaces[i]);
  }


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


  //////////////////////////////////////////////////////////////////////////////
  // Reordering cis_to_gis and gis_to_cis maps
  //////////////////////////////////////////////////////////////////////////////

  for(auto is: context_.coloring_map()) {
    size_t index_space = is.first;

    auto& _cis_to_gis = context_.cis_to_gis_map(index_space);
    auto& _gis_to_cis = context_.gis_to_cis_map(index_space);

    auto & _color_map = context_.coloring_info(index_space);

    std::vector<size_t> _rank_offsets(context_.colors()+1, 0);

    size_t offset = 0;

    for(size_t c{0}; c<context_.colors(); ++c) {
      auto & _color_info = _color_map.at(c);
      _rank_offsets[c] = offset;
      offset += (_color_info.exclusive + _color_info.shared);
    } // for

    size_t cid{0};
    for(auto entity: is.second.exclusive) {
      size_t gid = _rank_offsets[entity.rank] + entity.offset;
      _cis_to_gis[cid] = gid;
      _gis_to_cis[gid] = cid;
      ++cid;
    } // for

    for(auto entity: is.second.shared) {
      size_t gid = _rank_offsets[entity.rank] + entity.offset;
      _cis_to_gis[cid] = gid;
      _gis_to_cis[gid] = cid;
      ++cid;
    } // for

    for(auto entity: is.second.ghost) {
      size_t gid = _rank_offsets[entity.rank] + entity.offset;
      _cis_to_gis[cid] = gid;
      _gis_to_cis[gid] = cid;
      ++cid;
    } // for

  } // for

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  // Get the input arguments from the Legion runtime
  const Legion::InputArgs & args =
    Legion::Runtime::get_input_args();

/*  //adding information for the global and color handles to the ispace_map
  if (number_of_global_fields>0){

    size_t global_index_space =
      execution::internal_index_space::global_is;

    ispace_dmap[global_index_space].color_region =
        regions[region_index].get_logical_region();

    region_index++;
  }//end if

  if(number_of_color_fields>0){

    size_t color_index_space =
      execution::internal_index_space::color_is;
*/


  if(number_of_sparse_fields > 0){
    context_t::sparse_field_data_t md;

    Legion::PhysicalRegion pr = regions[region_index];
    Legion::LogicalRegion lr = pr.get_logical_region();
    Legion::IndexSpace is = lr.get_index_space();

    for(const field_info_t& fi : context_.registered_fields()){
      if(fi.storage_class != data::sparse && fi.storage_class != data::ragged){
        continue;
      }

      size_t idx_space = fi.index_space;
      
      auto si = sis_map.find(idx_space);

      using sparse_field_data_t = context_t::sparse_field_data_t;
      using coloring_info_t = context_t::coloring_info_t;
    
      const auto& cim = context_.coloring_info(idx_space);
      auto citr = cim.find(my_color);
      const coloring_info_t& ci = citr->second;

     //Metadata LR is the first and only region passed to this task
     Legion::PhysicalRegion pr = regions[region_index];
     Legion::LogicalRegion lr = pr.get_logical_region();
     Legion::IndexSpace is = lr.get_index_space();

     auto ac = pr.get_field_accessor(fi.fid).
        template typeify<sparse_field_data_t>();

      Legion::Domain domain = runtime->get_index_space_domain(ctx, is);

      LegionRuntime::Arrays::Rect<2> dr = domain.get_rect<2>();
      LegionRuntime::Arrays::Rect<2> sr;
      LegionRuntime::Accessor::ByteOffset bo[2];
      sparse_field_data_t* metadata = ac.template raw_rect_ptr<2>(dr, sr, bo);
      *metadata = sparse_field_data_t(fi.size, ci.exclusive,
        ci.shared,
        ci.ghost,
        si->second.max_entries_per_index, si->second.exclusive_reserve);

      
/*      md = sparse_field_data_t(fi.size, ci.exclusive,
        ci.shared,
        ci.ghost,
        si->second.max_entries_per_index, si->second.exclusive_reserve);
*/
//      context_.set_sparse_metadata(md);
    }//for
    region_index++;

  }//if 

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

  delete [] field_info_buf;
  delete [] adjacencies;
  delete [] index_subspaces;
  delete [] sparse_index_spaces;

} // setup_rank_context_task


} // namespace execution 
} // namespace flecsi
