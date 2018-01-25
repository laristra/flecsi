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

    ispace_dmap[global_index_space].color_region =
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

  for(auto& itr : context_.index_subspace_info()){
    data.add_index_subspace(itr.second);
  }

  data.finalize(coloring_info);

  //-------------------------------------------------------------------------//
  //  Create Legion reduction 
  //-------------------------------------------------------------------------//

  double min = std::numeric_limits<double>::min();
  Legion::DynamicCollective max_reduction =
  runtime->create_dynamic_collective(ctx, num_colors, MaxReductionOp::redop_id,
             &min, sizeof(min));

  double max = std::numeric_limits<double>::max();
  Legion::DynamicCollective min_reduction =
  runtime->create_dynamic_collective(ctx, num_colors, MinReductionOp::redop_id,
             &max, sizeof(max));

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
        flecsi_ispace.logical_region, flecsi_ispace.index_partition);
        pos_compaction_launcher.add_region_requirement(
        Legion::RegionRequirement(color_lpart, 0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);
  } // for idx_space

  Legion::MustEpochLauncher must_epoch_launcher1;
   must_epoch_launcher1.add_index_task(pos_compaction_launcher);
  auto fm = runtime->execute_must_epoch(ctx, must_epoch_launcher1);

  fm.wait_all_results(true);

  
  //--------------------------------------------------------------------------//
  //  Create Phase barriers per each Field
  //-------------------------------------------------------------------------//

  // map of index space to the field_ids that are mapped to this index space
  std::map<size_t, std::vector<field_id_t>> fields_map;

  //total number of Phase Barriers 
  size_t num_phase_barriers =0;

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
            num_phase_barriers++;
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


  // the key is index_space id, internal map key if field id 
  std::map<size_t, std::map<field_id_t, std::vector<Legion::PhaseBarrier>>>
      phase_barriers_map;

  //fill the map
  for(auto idx_space : coloring_info){
    std::map<field_id_t , std::vector<Legion::PhaseBarrier>> inner;
    for (const field_id_t& field_id : fields_map[idx_space.first]){
      for(int color = 0; color < num_colors; color++){
        const flecsi::coloring::coloring_info_t& color_info = 
          idx_space.second[color];

        inner[field_id].push_back(runtime->create_phase_barrier(ctx,
             1 + color_info.shared_users.size()));
      }//color
    }//field_info
    phase_barriers_map[idx_space.first]=inner;
  }//indx_space

  //--------------------------------------------------------------------------//
  //   Create Legion must epoch launcher and add Region requirements
  //-------------------------------------------------------------------------//
  
  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;

  std::map<size_t,Legion::Serializer> args_serializers;

  const auto spmd_id =
    context_.task_id<__flecsi_internal_task_key(spmd_task)>();

  {
  clog_tag_guard(runtime_driver);
  clog(trace) << "spmd_task is: " << spmd_id << std::endl;
  } // scope


  // Add colors to must_epoch_launcher
  for(size_t color(0); color<num_colors; ++color) {

    // Serialize PhaseBarriers and set as task arguments
    std::vector<Legion::PhaseBarrier> pbarriers_as_owner;
    std::vector<size_t> num_ghost_owners;
    std::map<size_t, std::map<field_id_t, std::vector<Legion::PhaseBarrier>>>
      owners_pbarriers;

    for(auto is: context_.coloring_map()) {
      size_t idx_space = is.first;

      flecsi::coloring::coloring_info_t color_info =
          coloring_info[idx_space][color];

      for (const field_id_t& field_id : fields_map[idx_space]){
        pbarriers_as_owner.push_back(
          phase_barriers_map[idx_space][field_id][color]);
      
        {
        clog_tag_guard(runtime_driver);
        clog(trace) << " Color " << color << " idx_space " << idx_space 
        << ", fid = " << field_id<<
          " has " << color_info.ghost_owners.size() << 
          " ghost owners" << std::endl;
        } // scope

        for(auto owner : color_info.ghost_owners) {
          {
          clog_tag_guard(runtime_driver);
          clog(trace) << owner << std::endl;
          } // scope

          owners_pbarriers[idx_space][field_id].push_back(
            phase_barriers_map[idx_space][field_id][owner]);
       
        }
      
      }//for field_info

      num_ghost_owners.push_back(color_info.ghost_owners.size());
    } // for idx_space

    size_t num_idx_spaces = context_.coloring_map().size();
   
    //-----------------------------------------------------------------------//
    // data serialization:
    //-----------------------------------------------------------------------//
    
    // #1 serialize num_indx_spaces & num_phase_barriers
    args_serializers[color].serialize(&num_idx_spaces, sizeof(size_t));
    args_serializers[color].serialize(&num_phase_barriers, sizeof(size_t));
    args_serializers[color].serialize(&number_of_global_fields, sizeof(size_t));
    args_serializers[color].serialize(&number_of_color_fields, sizeof(size_t));
   

    // #2 serialize field info
    size_t num_fields = context_.registered_fields().size();
    args_serializers[color].serialize(&num_fields, sizeof(size_t));
    args_serializers[color].serialize(
      &context_.registered_fields()[0], num_fields * sizeof(field_info_t));

    // #3 serialize pbarriers_as_owner
    args_serializers[color].serialize(&pbarriers_as_owner[0], 
      num_phase_barriers * sizeof(Legion::PhaseBarrier));

    // #4 serialize num_ghost_owners[
    args_serializers[color].serialize(&num_ghost_owners[0], num_idx_spaces
        * sizeof(size_t));

    // #5 serialize owners_pbarriers
    std::vector <Legion::PhaseBarrier> owners_pbarriers_buf;
    for(auto is: context_.coloring_map()) {
      size_t idx_space = is.first;
      for (const field_id_t& field_id : fields_map[idx_space])
        for(auto pb:owners_pbarriers[idx_space][field_id])
           owners_pbarriers_buf.push_back(pb);
    }//for
  
    size_t num_owners_pbarriers = owners_pbarriers_buf.size();
    args_serializers[color].serialize(&num_owners_pbarriers, sizeof(size_t));
    args_serializers[color].serialize(&owners_pbarriers_buf[0],
      num_owners_pbarriers * sizeof(Legion::PhaseBarrier));

    // #6 serialize reduction
    args_serializers[color].serialize(&max_reduction,
        sizeof(Legion::DynamicCollective));
    args_serializers[color].serialize(&min_reduction,
        sizeof(Legion::DynamicCollective));

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
   //add region requirements to the spmd_launcher
   //-----------------------------------------------------------------------//
   
    Legion::TaskLauncher spmd_launcher(spmd_id,
        Legion::TaskArgument(args_serializers[color].get_buffer(),
                             args_serializers[color].get_used_bytes()));
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    // Add region requirements
    for(auto is: context_.coloring_map()) {
      size_t idx_space = is.first;
      auto& flecsi_ispace = data.index_space(idx_space);

      Legion::LogicalPartition color_lpart =
        runtime->get_logical_partition(ctx,
          flecsi_ispace.logical_region, flecsi_ispace.index_partition);
      
      Legion::LogicalRegion color_lregion =
        runtime->get_logical_subregion_by_color(ctx, color_lpart, color);

      Legion::RegionRequirement reg_req(color_lregion, READ_WRITE,
          SIMULTANEOUS, flecsi_ispace.logical_region);

      reg_req.add_field(ghost_owner_pos_fid);

      for (const field_id_t& field_id : fields_map[idx_space]){
          reg_req.add_field(field_id);
      }//for field_info

      for(auto& itr : context_.adjacency_info()){
        if(itr.first == idx_space){
          Legion::FieldID adjacency_fid = 
            context_.adjacency_fid(itr.second.from_index_space,
              itr.second.to_index_space);
          
          reg_req.add_field(adjacency_fid);
        }
      }

      spmd_launcher.add_region_requirement(reg_req);

      flecsi::coloring::coloring_info_t color_info = 
        coloring_info[idx_space][color];
      
      for(auto ghost_owner : color_info.ghost_owners) {
        {
        clog_tag_guard(runtime_driver);
        clog(trace) << " Color " << color << " idx_space " << idx_space << 
          " has owner " << ghost_owner << std::endl;
        } // scope

        Legion::LogicalRegion ghost_owner_lregion =
          runtime->get_logical_subregion_by_color(ctx, color_lpart,
            ghost_owner);

        const LegionRuntime::Arrays::coord_t owner_color = ghost_owner;
        const bool is_mutable = false;
        runtime->attach_semantic_information(ghost_owner_lregion,
          OWNER_COLOR_TAG, (void*)&owner_color,
          sizeof(LegionRuntime::Arrays::coord_t), is_mutable);

        Legion::RegionRequirement owner_reg_req(ghost_owner_lregion, READ_ONLY,
          SIMULTANEOUS, flecsi_ispace.logical_region);
        owner_reg_req.add_flags(NO_ACCESS_FLAG);
        owner_reg_req.add_field(ghost_owner_pos_fid);
        for (const field_id_t& field_id : fields_map[idx_space]){
          owner_reg_req.add_field(field_id);
        }
        spmd_launcher.add_region_requirement(owner_reg_req);

      }// for ghost_owner

    } // for idx_space

    for(size_t adjacency_idx_space : data.adjacencies()){
      auto& adjacency = data.adjacency(adjacency_idx_space);

      Legion::LogicalPartition color_lpart =
        runtime->get_logical_partition(ctx,
          adjacency.logical_region, adjacency.index_partition);
      
      Legion::LogicalRegion color_lregion =
        runtime->get_logical_subregion_by_color(ctx, color_lpart, color);

      Legion::RegionRequirement
        reg_req(color_lregion, READ_WRITE, SIMULTANEOUS,
          adjacency.logical_region);

      for(const field_info_t& fi : context_.registered_fields()){
        if(fi.index_space == adjacency_idx_space){
          reg_req.add_field(fi.fid);
        }
      }

      spmd_launcher.add_region_requirement(reg_req);
    }//adjacency_indx

    for(auto& itr : data.index_subspace_map()){
      const data::legion_data_t::index_subspace_t& info = itr.second;

      Legion::RegionRequirement
        reg_req(info.logical_region, READ_WRITE, SIMULTANEOUS,
          info.logical_region);
        reg_req.add_field(info.fid);
    }

    auto global_ispace = data.global_index_space();
    Legion::RegionRequirement global_reg_req(global_ispace.logical_region,
          READ_ONLY, SIMULTANEOUS, global_ispace.logical_region);

    global_reg_req.add_flags(NO_ACCESS_FLAG);
    for(const field_info_t& field_info : context_.registered_fields()){
      if(field_info.storage_class == data::global ){
         global_reg_req.add_field(field_info.fid);
       }//if
     }//for

     if (number_of_global_fields>0)
       spmd_launcher.add_region_requirement(global_reg_req);

    auto color_ispace = data.color_index_space();

    Legion::LogicalPartition color_lp = runtime->get_logical_partition(ctx,
        color_ispace.logical_region, color_ispace.index_partition);

    Legion::LogicalRegion color_lregion2 =
        runtime->get_logical_subregion_by_color(ctx, color_lp, color);

    Legion::RegionRequirement color_reg_req(color_lregion2,
          READ_WRITE, SIMULTANEOUS, color_ispace.logical_region);

   // color_reg_req.add_flags(NO_ACCESS_FLAG);
    for(const field_info_t& field_info : context_.registered_fields()){
       if(field_info.storage_class == data::color ){
         color_reg_req.add_field(field_info.fid);
       }//if
     }//for

     if (number_of_color_fields>0)
       spmd_launcher.add_region_requirement(color_reg_req);

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, spmd_launcher);
  } // for color

  // Launch the spmd tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  bool silence_warnings = true;
  future.wait_all_results(silence_warnings);

  //-----------------------------------------------------------------------//
  // Finish up Legion runtime and fall back out to MPI.
  // ----------------------------------------------------------------------//

  runtime->destroy_dynamic_collective(ctx, max_reduction);
  runtime->destroy_dynamic_collective(ctx, min_reduction);

  for(auto& itr_idx : phase_barriers_map) {
    const size_t idx = itr_idx.first;
    for(auto& itr_fid : phase_barriers_map[idx]) {
      const field_id_t fid = itr_fid.first;
      for(size_t color = 0; color < phase_barriers_map[idx][fid].size();
        color ++) {
        runtime->destroy_phase_barrier(ctx,
          phase_barriers_map[idx][fid][color]);
      }
      phase_barriers_map[idx][fid].clear();
    }
    phase_barriers_map[idx].clear();
  }
  phase_barriers_map.clear();

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // runtime_driver

void
spmd_task(
  const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime
)
{
  using namespace data;

  const int my_color = task->index_point.point_data[0];

  // spmd_task is an inner task
  runtime->unmap_all_regions(ctx);

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing spmd task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();
  context_.advance_state();

  auto& ispace_dmap = context_.index_space_data_map();

  auto ghost_owner_pos_fid = 
    Legion::FieldID(internal_field::ghost_owner_pos);

  clog_assert(task->arglen > 0, "spmd_task called without arguments");

  //---------------------------------------------------------------------//
  // Deserialize task arguments
  // --------------------------------------------------------------------//

  Legion::Deserializer args_deserializer(task->args, task->arglen);
 
  //#1 serialize num_indx_spaces & num_phase_barriers
  size_t num_idx_spaces;
  size_t num_phase_barriers;
  size_t number_of_global_fields;
  size_t number_of_color_fields;
  args_deserializer.deserialize(&num_idx_spaces, sizeof(size_t));
  args_deserializer.deserialize(&num_phase_barriers, sizeof(size_t));
  args_deserializer.deserialize(&number_of_global_fields, sizeof(size_t));
  args_deserializer.deserialize(&number_of_color_fields, sizeof(size_t));

  {
  size_t total_num_idx_spaces = num_idx_spaces;
  if (number_of_global_fields>0) total_num_idx_spaces++;
  if (number_of_color_fields>0) total_num_idx_spaces++;

  clog_assert(regions.size() >= (total_num_idx_spaces),
      "fewer regions than data handles");
  clog_assert(task->regions.size() >= (total_num_idx_spaces),
      "fewer regions than data handles");
  }//scope

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

  //if there is no information about fields in the context, add it there
  if (context_.registered_fields().size()==0)
  {
    for(size_t i = 0; i < num_fields; ++i){
      field_info_t& fi = field_info_buf[i];
      context_.register_field_info(fi);
    }
  }

 // map of index space to the field_ids that are mapped to this index space
  std::map<size_t, std::vector<field_id_t>> fields_map;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
      if((field_info.storage_class != global) &&
        (field_info.storage_class != color)){
        if(field_info.index_space == idx_space){
          fields_map[idx_space].push_back(field_info.fid);
        }
      }//if
    }//for
  }//end for is

  // #3 deserialize pbarriers_as_owner
  Legion::PhaseBarrier* pbarriers_as_owner =
    new Legion::PhaseBarrier [num_phase_barriers];
  args_deserializer.deserialize((void*)pbarriers_as_owner,
      sizeof(Legion::PhaseBarrier) * num_phase_barriers);

  //for (size_t i=0; i<num_phase_barriers;i++ )
  //  {
  //  clog_tag_guard(runtime_driver);
  //  clog(trace) <<my_color <<" has pbarrier_as_owner "<<
	//		pbarriers_as_owner[i]<<std::endl;
  //  } // scope

  // #4 deserialize num_ghost_owners[
  size_t* num_owners = new size_t [num_idx_spaces];
  args_deserializer.deserialize((void*)num_owners, sizeof(size_t)
      * num_idx_spaces);

  // fille index_space_data_map with pbarriers_as_owner
  size_t indx = 0;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for (const field_id_t& field_id : fields_map[idx_space]){
      ispace_dmap[idx_space].pbarriers_as_owner[field_id] =
        pbarriers_as_owner[indx];
      ispace_dmap[idx_space].ghost_is_readable[field_id] = true;
      ispace_dmap[idx_space].write_phase_started[field_id] = false;
      indx++;
    }//end field_info
  }//end for idx_space

  //#5 Deserialize ghost_owners_pbarriers
  size_t num_owners_pbarriers;
  args_deserializer.deserialize(&num_owners_pbarriers, sizeof(size_t));

  Legion::PhaseBarrier* ghost_owners_pbarriers =
    new Legion::PhaseBarrier [num_owners_pbarriers];
  
  args_deserializer.deserialize((void*)ghost_owners_pbarriers,
    sizeof(Legion::PhaseBarrier) * num_owners_pbarriers);

  // fill index_space_data_map with ghost_owners_pbarriers
  indx=0;
  size_t consec_indx = 0;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    size_t n = num_owners[consec_indx];
    for (const field_id_t& field_id : fields_map[idx_space]){
       ispace_dmap[idx_space].ghost_owners_pbarriers[field_id].resize(n);

       for(size_t owner = 0; owner < n; ++owner){
         ispace_dmap[idx_space].ghost_owners_pbarriers[field_id][owner] =
            ghost_owners_pbarriers[indx];
         indx++;
         {
         clog_tag_guard(runtime_driver);
         clog(trace) <<my_color <<" has ghost_owners_pbarrier "<<
             ghost_owners_pbarriers[indx-1]<<std::endl;
         } // scope
      }//owner
    }//field_id
    consec_indx++;
  }//idx_space

  // Prevent these objects destructors being called until after driver()
  std::map<size_t, std::vector<Legion::LogicalRegion>>
    ghost_owners_lregions;
  std::map<size_t, std::vector<Legion::LogicalRegion>>
    ghost_owners_subregions;
  std::map<size_t,Legion::IndexPartition> primary_ghost_ips;
  std::map<size_t,Legion::IndexPartition> exclusive_shared_ips;
  std::map<size_t,std::map<size_t,Legion::IndexPartition>> owner_subrect_ips;

  //fill ispace_dmap with logical regions
  size_t region_index = 0;
  size_t consecutive_index = 0;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    
    ispace_dmap[idx_space].color_region = regions[region_index]
                                                  .get_logical_region();

    const std::unordered_map<size_t, flecsi::coloring::coloring_info_t>
      coloring_info_map = context_.coloring_info(idx_space);

    auto itr = coloring_info_map.find(my_color);
    clog_assert(itr != coloring_info_map.end(),
        "Can't find partition info for my color");
    const flecsi::coloring::coloring_info_t coloring_info = itr->second;

    {
    clog_tag_guard(runtime_driver);
    clog(trace) << my_color << " handle " << idx_space <<
        " exclusive " << coloring_info.exclusive <<
        " shared " << coloring_info.shared <<
        " ghost " << coloring_info.ghost << std::endl;
    } // scope

    Legion::IndexSpace color_ispace = 
      regions[region_index].get_logical_region().get_index_space();
    LegionRuntime::Arrays::Rect<1> color_bounds_1D(0,1);
    Legion::Domain color_domain_1D
    = Legion::Domain::from_rect<1>(color_bounds_1D);

    Legion::DomainColoring primary_ghost_coloring;
    LegionRuntime::Arrays::Rect<2>
    primary_rect(LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive
            + coloring_info.shared - 1));
    primary_ghost_coloring[PRIMARY_PART]
                           = Legion::Domain::from_rect<2>(primary_rect);
    LegionRuntime::Arrays::Rect<2> ghost_rect(
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive
            + coloring_info.shared),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive
            + coloring_info.shared + coloring_info.ghost - 1));
    primary_ghost_coloring[GHOST_PART]
                           = Legion::Domain::from_rect<2>(ghost_rect);

    Legion::IndexPartition primary_ghost_ip =
      runtime->create_index_partition(ctx, color_ispace, color_domain_1D,
      primary_ghost_coloring, true /*disjoint*/);

    primary_ghost_ips[idx_space] = primary_ghost_ip;

    Legion::LogicalPartition primary_ghost_lp =
      runtime->get_logical_partition(ctx,
        regions[region_index].get_logical_region(), primary_ghost_ip);
    region_index++;

    Legion::LogicalRegion primary_lr =
    runtime->get_logical_subregion_by_color(ctx, primary_ghost_lp, 
                                            PRIMARY_PART);

    ispace_dmap[idx_space].primary_lr = primary_lr;

    ispace_dmap[idx_space].ghost_lr = 
      runtime->get_logical_subregion_by_color(ctx, primary_ghost_lp, 
                                              GHOST_PART);

    Legion::DomainColoring excl_shared_coloring;
    LegionRuntime::Arrays::Rect<2> exclusive_rect(
        LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color,
          coloring_info.exclusive - 1));
    excl_shared_coloring[EXCLUSIVE_PART]
                         = Legion::Domain::from_rect<2>(exclusive_rect);
    LegionRuntime::Arrays::Rect<2> shared_rect(
        LegionRuntime::Arrays::make_point(my_color,
        coloring_info.exclusive),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive
            + coloring_info.shared - 1));
    excl_shared_coloring[SHARED_PART]
                         = Legion::Domain::from_rect<2>(shared_rect);

    Legion::IndexPartition excl_shared_ip = runtime->create_index_partition(ctx,
        primary_lr.get_index_space(), color_domain_1D, excl_shared_coloring,
        true /*disjoint*/);

    exclusive_shared_ips[idx_space] = excl_shared_ip;

    Legion::LogicalPartition excl_shared_lp
      = runtime->get_logical_partition(ctx, primary_lr, excl_shared_ip);

    ispace_dmap[idx_space].exclusive_lr = 
    runtime->get_logical_subregion_by_color(ctx, excl_shared_lp, 
                                            EXCLUSIVE_PART);

    ispace_dmap[idx_space].shared_lr = 
    runtime->get_logical_subregion_by_color(ctx, excl_shared_lp, SHARED_PART);

    // Add neighbors regions to context_
    ghost_owners_subregions[idx_space].resize(num_owners[consecutive_index]);
    for(size_t owner = 0; owner < num_owners[consecutive_index]; owner++) {
      ghost_owners_lregions[idx_space].push_back(regions[region_index]
        .get_logical_region());
      const void* owner_color;
      size_t size;
      const bool can_fail = false;
      const bool wait_until_ready = true;

      clog_assert(region_index < regions.size(),
          "SPMD attempted to access more regions than passed");

      runtime->retrieve_semantic_information(regions[region_index]
          .get_logical_region(), OWNER_COLOR_TAG,
          owner_color, size, can_fail, wait_until_ready);
      clog_assert(size == sizeof(LegionRuntime::Arrays::coord_t),
          "Unable to map gid to lid with Legion semantic tag");
      ispace_dmap[idx_space]
       .global_to_local_color_map[*(LegionRuntime::Arrays::coord_t*)owner_color]
       = owner;

      {
      clog_tag_guard(runtime_driver);
      clog(trace) << my_color << " key " << idx_space << " gid " <<
          *(LegionRuntime::Arrays::coord_t*)owner_color <<
          " maps to " << owner << std::endl;
      } // scope

      region_index++;
      clog_assert(region_index <= regions.size(),
          "SPMD attempted to access more regions than passed");
    } // for owner

    ispace_dmap[idx_space].ghost_owners_lregions
      = ghost_owners_lregions[idx_space];


    // Fix ghost reference/pointer to point to compacted position of
    // shared that it needs
    Legion::TaskLauncher fix_ghost_refs_launcher(context_.task_id<
      __flecsi_internal_task_key(owner_pos_correction_task)>(),
      Legion::TaskArgument(nullptr, 0));

    {
    clog_tag_guard(runtime_driver);
    clog(trace) << "Rank" << my_color << " Index " << idx_space <<
      " RW " << ispace_dmap[idx_space].color_region << std::endl;
    } // scope

    fix_ghost_refs_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_lr, READ_WRITE,
            EXCLUSIVE, ispace_dmap[idx_space].color_region)
        .add_field(ghost_owner_pos_fid));

    fix_ghost_refs_launcher.add_future(Legion::Future::from_value(runtime,
            ispace_dmap[idx_space].global_to_local_color_map));

    for(size_t owner = 0; owner < num_owners[consecutive_index]; owner++)
      fix_ghost_refs_launcher.add_region_requirement(
         Legion::RegionRequirement(ghost_owners_lregions[idx_space][owner],
              READ_ONLY, EXCLUSIVE, ghost_owners_lregions[idx_space][owner])
              .add_field(ghost_owner_pos_fid));

    runtime->execute_task(ctx, fix_ghost_refs_launcher);

    // Find subrects from each owner that I must copy in ghost_copy_task
    Legion::TaskLauncher owners_subregions_launcher(context_.task_id<
      __flecsi_internal_task_key(owners_subregions_task)>(),
      Legion::TaskArgument(nullptr, 0));

    owners_subregions_launcher.add_future(Legion::Future::from_value(runtime,
            ispace_dmap[idx_space].global_to_local_color_map));

    owners_subregions_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_lr, READ_ONLY,
            EXCLUSIVE, ispace_dmap[idx_space].color_region)
        .add_field(ghost_owner_pos_fid));

    Legion::Future owner_to_subrect_future =
        runtime->execute_task(ctx, owners_subregions_launcher);

    bool silence_warnings = true;   // more efficient to defer this to just
                                    // before calling driver, but this is
                                    // only a one-time setup
    subrect_map owner_to_subrect_map =
        owner_to_subrect_future.get_result<subrect_map>(silence_warnings);

    for(auto owner_itr=owner_to_subrect_map.begin();
        owner_itr!=owner_to_subrect_map.end(); owner_itr++) {
      size_t owner = owner_itr->first;
      LegionRuntime::Arrays::Rect<2> sub_rect = owner_itr->second;

      Legion::IndexSpace color_ispace =
          ghost_owners_lregions[idx_space][owner].get_index_space();
      LegionRuntime::Arrays::Rect<1> color_bounds_1D(0,1);
      Legion::Domain color_domain_1D
      = Legion::Domain::from_rect<1>(color_bounds_1D);

      Legion::DomainColoring owner_subrect_coloring;
      owner_subrect_coloring[SUBRECT_PART]
                             = Legion::Domain::from_rect<2>(sub_rect);
      Legion::IndexPartition owner_subrect_ip =
        runtime->create_index_partition(ctx, color_ispace, color_domain_1D,
        owner_subrect_coloring, true /*disjoint*/);

      //auto ips_itr = owner_subrect_ips.find(owner);
      //if (ips_itr == owner_subrect_ips.end())
       // owner_subrect_ips[owner].resize(num_idx_spaces);
      owner_subrect_ips[owner][idx_space] = owner_subrect_ip;

      Legion::LogicalPartition owner_subrect_lp =
        runtime->get_logical_partition(ctx,
            ghost_owners_lregions[idx_space][owner], owner_subrect_ip);

      Legion::LogicalRegion subrect_lr =
      runtime->get_logical_subregion_by_color(ctx, owner_subrect_lp,
                                              SUBRECT_PART);

      ghost_owners_subregions[idx_space][owner] = subrect_lr;

    } //owner_itr
    ispace_dmap[idx_space].ghost_owners_subregions
      = ghost_owners_subregions[idx_space];

    consecutive_index++;
  } // for idx_space
  

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
  } // for

  //////////////////////////////////////////////////////////////////////////////
  // FIX for Legion cluster fuck reordering nightmare...
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

  // #6 deserialize reduction
  Legion::DynamicCollective max_reduction;
  args_deserializer.deserialize((void*)&max_reduction,
    sizeof(Legion::DynamicCollective));
  context_.set_max_reduction(max_reduction);
  
  Legion::DynamicCollective min_reduction;
  args_deserializer.deserialize((void*)&min_reduction,
    sizeof(Legion::DynamicCollective));
  context_.set_min_reduction(min_reduction);

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

  for(auto& itr : context_.adjacencies()) {
    ispace_dmap[itr.first].color_region = 
      regions[region_index].get_logical_region();

    region_index++;
  }

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

  auto& isubspace_dmap = context_.index_subspace_data_map();

  for(auto& itr : context_.index_subspace_info()) {
    isubspace_dmap[itr.first].region = 
      regions[region_index].get_logical_region();

    region_index++;
  }

  //adding information for the global and color handles to the ispace_map
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

    ispace_dmap[color_index_space].color_region =
      regions[region_index].get_logical_region();  
  }//end if

  // Call the specialization color initialization function.
#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
  specialization_spmd_init(args.argc, args.argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

  context_.advance_state();

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

  // run default or user-defined driver
  driver(args.argc, args.argv);

  // Cleanup memory
  for(auto ipart: primary_ghost_ips)
      runtime->destroy_index_partition(ctx, ipart.second);
  for(auto ipart: exclusive_shared_ips)
      runtime->destroy_index_partition(ctx, ipart.second);
  delete [] field_info_buf;
  delete [] num_owners;
  delete [] pbarriers_as_owner;
  delete [] adjacencies;
  delete [] index_subspaces;
//  delete [] idx_spaces;

} // spmd_task

} // namespace execution 
} // namespace flecsi
