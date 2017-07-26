/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include "flecsi/execution/legion/runtime_driver.h"

#include <legion.h>
#include <legion_utilities.h>
#include <limits>

#include "flecsi/data/storage.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/legion_tasks.h"
#include "flecsi/execution/legion/mapper.h"
#include "flecsi/execution/legion/internal_field.h"
#include "flecsi/data/legion/legion_data.h"
#include "flecsi/utils/common.h"

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
  using field_id_t = Legion::FieldID;

#if defined FLECSI_ENABLE_SPECIALIZATION_TLT_INIT
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization top-level-task init" << std::endl;
  }

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_.push_state(utils::const_string_t{"specialization_tlt_init"}.hash(),
    ctx, runtime, task, regions);
#endif

  // Invoke the specialization top-level task initialization function.
  specialization_tlt_init(args.argc, args.argv);

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_.pop_state( utils::const_string_t{"specialization_tlt_init"}.hash());
#endif

#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT

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

  auto coloring_info = context_.coloring_info_map();


  data::legion_data_t data(ctx, runtime, num_colors);

  data.init_from_coloring_info_map(coloring_info);

  for(auto& itr : context_.adjacency_info()){
    data.add_adjacency(itr.second);
  }

  data.finalize(coloring_info);

  std::map<size_t, std::vector<field_id_t>> fields_map;

  size_t num_phase_barriers =0;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
        if(field_info.index_space == idx_space){
          fields_map[idx_space].push_back(field_info.fid);
          num_phase_barriers++;
        } // if
      } // for

      {
      clog_tag_guard(runtime_driver);
      clog(trace) << "fields_map[" <<idx_space<<"] has "<<
        fields_map[idx_space].size()<< " fields"<<std::endl;
      } // scope
  } // for

  std::map<size_t, std::map<field_id_t, std::vector<Legion::PhaseBarrier>>>
    phase_barriers_map;

  double min = std::numeric_limits<double>::min();
  Legion::DynamicCollective max_reduction =
  runtime->create_dynamic_collective(ctx, num_colors, MaxReductionOp::redop_id,
             &min, sizeof(min));

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

  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);
 
  // Map between pre-compacted and compacted data placement
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
  
  runtime->execute_index_space(ctx, pos_compaction_launcher);

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
      
        clog(trace) << " Color " << color << " idx_space " << idx_space 
        << ", fid = " << field_id<<
          " has " << color_info.ghost_owners.size() << 
          " ghost owners" << std::endl;

        for(auto owner : color_info.ghost_owners) {
          clog(trace) << owner << std::endl;
          owners_pbarriers[idx_space][field_id].push_back(
            phase_barriers_map[idx_space][field_id][owner]);
       
        }
      
      }//for field_info
      num_ghost_owners.push_back(color_info.ghost_owners.size());
    } // for idx_space

    size_t num_idx_spaces = context_.coloring_map().size();

   //data serialization:

    // #1 serialize num_indx_spaces & num_phase_barriers
    args_serializers[color].serialize(&num_idx_spaces, sizeof(size_t));
    args_serializers[color].serialize(&num_phase_barriers, sizeof(size_t));

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

    args_serializers[color].serialize(&max_reduction,
        sizeof(Legion::DynamicCollective));

  using adjacency_triple_t = context_t::adjacency_triple_t;
  
  std::vector<adjacency_triple_t> adjacencies_vec;
  
  for(auto& itr : context_.adjacency_info()){
    const coloring::adjacency_info_t& ai = itr.second;
    auto t = std::make_tuple(ai.index_space, ai.from_index_space,
      ai.to_index_space);
    adjacencies_vec.push_back(t);
  }

  size_t num_adjacencies = adjacencies_vec.size();

  args_serializers[color].serialize(&num_adjacencies, sizeof(size_t));
  args_serializers[color].serialize(&adjacencies_vec[0], num_adjacencies
    * sizeof(adjacency_triple_t));

   //add region requirements to the spmd_launcher
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

      for(const field_info_t& field_info : context_.registered_fields()){
        if(field_info.index_space == idx_space){
          reg_req.add_field(field_info.fid);
        }
      }

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
        for(const field_info_t& field_info : context_.registered_fields()){
          if(field_info.index_space == idx_space){
            owner_reg_req.add_field(field_info.fid);
          }
        }

        spmd_launcher.add_region_requirement(owner_reg_req);
      } // for ghost_owner

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
    }

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, spmd_launcher);
  } // for color

  // Launch the spmd tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  future.wait_all_results();

  // Finish up Legion runtime and fall back out to MPI.

  //FIXME runtime->destroy_dynamic_collective(ctx, max_reduction);

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
  using field_id_t = Legion::FieldID;

  const int my_color = task->index_point.point_data[0];

  // spmd_task is an inner task
  runtime->unmap_all_regions(ctx);

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing spmd task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  auto& ispace_dmap = context_.index_space_data_map();

  auto ghost_owner_pos_fid = 
    Legion::FieldID(internal_field::ghost_owner_pos);

  clog_assert(task->arglen > 0, "spmd_task called without arguments");

  Legion::Deserializer args_deserializer(task->args, task->arglen);
  size_t num_idx_spaces;
  size_t num_phase_barriers;
  args_deserializer.deserialize(&num_idx_spaces, sizeof(size_t));
  args_deserializer.deserialize(&num_phase_barriers, sizeof(size_t));

  clog_assert(regions.size() >= num_idx_spaces,
      "fewer regions than data handles");
  clog_assert(task->regions.size() >= num_idx_spaces,
      "fewer regions than data handles");

  // Deserialize field info
  size_t num_fields;
  args_deserializer.deserialize(&num_fields, sizeof(size_t));

  using field_info_t = context_t::field_info_t;
  auto field_info_buf = new field_info_t [num_fields];

  args_deserializer.deserialize(field_info_buf,
                                sizeof(field_info_t) * num_fields);

  for(size_t i = 0; i < num_fields; ++i){
    field_info_t& fi = field_info_buf[i];
    context_.put_field_info(fi);
  }//end for i

  if (context_.registered_fields().size()==0)
  {
    for(size_t i = 0; i < num_fields; ++i){
      field_info_t& fi = field_info_buf[i];
      context_.register_field_info(fi);
    }
  }

  //the key is IS
  std::map<size_t, std::vector<field_id_t>> fields_map;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
        if(field_info.index_space == idx_space){
          fields_map[idx_space].push_back(field_info.fid);
        }
      }
  }//end for is


  Legion::PhaseBarrier* pbarriers_as_owner =
    new Legion::PhaseBarrier [num_phase_barriers];
  args_deserializer.deserialize((void*)pbarriers_as_owner,
      sizeof(Legion::PhaseBarrier) * num_phase_barriers);

  for (size_t i=0; i<num_phase_barriers;i++ ) 
    {
    clog_tag_guard(runtime_driver);
    clog(trace) <<my_color <<" has pbarrier_as_owner "<<
			pbarriers_as_owner[i]<<std::endl;
    } // scope

  size_t* num_owners = new size_t [num_idx_spaces];
  args_deserializer.deserialize((void*)num_owners, sizeof(size_t)
      * num_idx_spaces);

  size_t indx = 0;
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for (const field_id_t& field_id : fields_map[idx_space]){
      ispace_dmap[idx_space].pbarriers_as_owner[field_id] =
        pbarriers_as_owner[indx];
      indx++;
    }//end field_info
  }//end for idx_space

  // Deserialize ghost_owners_pbarriers
  size_t num_owners_pbarriers;
  args_deserializer.deserialize(&num_owners_pbarriers, sizeof(size_t));

  Legion::PhaseBarrier* ghost_owners_pbarriers =
    new Legion::PhaseBarrier [num_owners_pbarriers];

  args_deserializer.deserialize((void*)ghost_owners_pbarriers,
    sizeof(Legion::PhaseBarrier) * num_owners_pbarriers);

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
         clog(trace) <<my_color <<" has ghost_owners_pbarrier "<<
             ghost_owners_pbarriers[indx-1]<<std::endl;
      }//owner
    }//field_id
    consec_indx++;
  }//idx_space
  // Prevent these objects destructors being called until after driver()
  std::map<size_t, std::vector<Legion::LogicalRegion>>
    ghost_owners_lregions;
  std::vector<Legion::IndexPartition> primary_ghost_ips(num_idx_spaces);
  std::vector<Legion::IndexPartition> exclusive_shared_ips(num_idx_spaces);

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
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive - 1));
    excl_shared_coloring[EXCLUSIVE_PART]
                         = Legion::Domain::from_rect<2>(exclusive_rect);
    LegionRuntime::Arrays::Rect<2> shared_rect(
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive),
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


    // Fix ghost reference/pointer to point to compacted position of shared that it needs
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

  // Get the input arguments from the Legion runtime
  const Legion::InputArgs & args =
    Legion::Runtime::get_input_args();

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);
#endif

  Legion::DynamicCollective max_reduction;
  args_deserializer.deserialize((void*)&max_reduction,
    sizeof(Legion::DynamicCollective));
  context_.set_max_reduction(max_reduction);

  size_t num_adjacencies;
  args_deserializer.deserialize(&num_adjacencies, sizeof(size_t));
   
  using adjacency_triple_t = context_t::adjacency_triple_t;
  adjacency_triple_t* adjacencies = 
    (adjacency_triple_t*)malloc(sizeof(adjacency_triple_t) * num_adjacencies);
     
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

  // Call the specialization color initialization function.
#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
  specialization_spmd_init(args.argc, args.argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

  // run default or user-defined driver 
  driver(args.argc, args.argv); 

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());
#endif

  // Cleanup memory
  for(auto ipart: primary_ghost_ips)
      runtime->destroy_index_partition(ctx, ipart);
  for(auto ipart: exclusive_shared_ips)
      runtime->destroy_index_partition(ctx, ipart);
  delete [] field_info_buf;
  delete [] num_owners;
  delete [] pbarriers_as_owner;
//  delete [] idx_spaces;

} // spmd_task

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
