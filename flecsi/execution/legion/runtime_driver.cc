/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include "flecsi/execution/legion/runtime_driver.h"

#include <legion.h>
#include <legion_utilities.h>

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
  Legion::HighLevelRuntime * runtime
)
{
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "In Legion runtime driver" << std::endl;
  }

  // Get the input arguments from the Legion runtime
  const Legion::InputArgs & args =
    Legion::HighLevelRuntime::get_input_args();

  // Initialize MPI Interoperability
  context_t & context_ = context_t::instance();
  context_.connect_with_mpi(ctx, runtime);
  context_.wait_on_mpi(ctx, runtime);

  using field_info_t = context_t::field_info_t;

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization driver task" << std::endl;
  }

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_.push_state(utils::const_string_t{"specialization_driver"}.hash(),
    ctx, runtime, task, regions);
#endif

  // run default or user-defined specialization driver 
  specialization_driver(args.argc, args.argv);

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_.pop_state( utils::const_string_t{"specialization_driver"}.hash());
#endif
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER


  // Register user data, invokes callbacks to add field info into context
  data::storage_t::instance().register_all();

  auto & data_client_registry =
    flecsi::data::storage_t::instance().data_client_registry(); 

  // FIXME documentation required
  for(auto & c: data_client_registry) {
    for(auto & d: c.second) {
      d.second.second(d.second.first);
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

  std::map<size_t, std::vector<Legion::PhaseBarrier>> phase_barriers_map;

  for(auto idx_space : coloring_info){
    for(int color = 0; color < num_colors; color++){
      const flecsi::coloring::coloring_info_t& color_info = 
        idx_space.second[color];

      phase_barriers_map[idx_space.first].push_back(
        runtime->create_phase_barrier(ctx,
          1 + color_info.shared_users.size()));
      
      clog(trace) << "key " << idx_space.first << " phase barrier " << color <<
          " has " << color_info.shared_users.size() + 1 << " arrivers to " <<
          phase_barriers_map[idx_space.first].back() << std::endl;
    }
  }

  // Map between pre-compacted and compacted data placement
  const auto pos_compaction_id =
    context_.task_id<__flecsi_internal_task_key(owner_pos_compaction_task)>();
  
  Legion::IndexLauncher pos_compaction_launcher(pos_compaction_id,
      data.color_domain(), Legion::TaskArgument(nullptr, 0),
      Legion::ArgumentMap());
  
  pos_compaction_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  for(auto idx_space : data.index_spaces()) {
    auto& flecsi_ispace = data.index_space_info(idx_space);

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

  std::vector<Legion::Serializer> args_serializers(num_colors);

  const auto spmd_id =
    context_.task_id<__flecsi_internal_task_key(spmd_task)>();
  clog(trace) << "spmd_task is: " << spmd_id << std::endl;

  // Add colors to must_epoch_launcher
  for(size_t color(0); color<num_colors; ++color) {

    // Serialize PhaseBarriers and set as task arguments
    std::vector<Legion::PhaseBarrier> pbarriers_as_owner;
    std::vector<size_t> num_ghost_owners;
    std::vector<std::vector<Legion::PhaseBarrier>> owners_pbarriers;

    for(auto idx_space : data.index_spaces()) {
      pbarriers_as_owner.push_back(phase_barriers_map[idx_space][color]);
      
      flecsi::coloring::coloring_info_t color_info = 
        coloring_info[idx_space][color];
      
      clog(trace) << " Color " << color << " idx_space " << idx_space << 
        " has " << color_info.ghost_owners.size() << 
        " ghost owners" << std::endl;
      
      num_ghost_owners.push_back(color_info.ghost_owners.size());
      std::vector<Legion::PhaseBarrier> per_color_owners_pbs;
      for (auto owner : color_info.ghost_owners) {
        clog(trace) << owner << std::endl;
        per_color_owners_pbs.push_back(phase_barriers_map[idx_space][owner]);
      }
      
      owners_pbarriers.push_back(per_color_owners_pbs);
    } // for idx_space

    size_t num_idx_spaces = data.index_spaces().size();

    std::vector<size_t> idx_spaces_vec;
    idx_spaces_vec.insert(idx_spaces_vec.begin(),
      data.index_spaces().begin(), data.index_spaces().end());

    args_serializers[color].serialize(&num_idx_spaces, sizeof(size_t));
    args_serializers[color].serialize(&idx_spaces_vec[0], num_idx_spaces
        * sizeof(size_t));
    args_serializers[color].serialize(&pbarriers_as_owner[0], num_idx_spaces
        * sizeof(Legion::PhaseBarrier));
    args_serializers[color].serialize(&num_ghost_owners[0], num_idx_spaces
        * sizeof(size_t));
    for (size_t idx_space : data.index_spaces())
      args_serializers[color].serialize(&owners_pbarriers[idx_space][0],
          num_ghost_owners[idx_space] * sizeof(Legion::PhaseBarrier));

    size_t num_fields = context_.registered_fields().size();
    args_serializers[color].serialize(&num_fields, sizeof(size_t));
    args_serializers[color].serialize(
      &context_.registered_fields()[0], num_fields * sizeof(field_info_t));

    Legion::TaskLauncher spmd_launcher(spmd_id,
        Legion::TaskArgument(args_serializers[color].get_buffer(),
                             args_serializers[color].get_used_bytes()));
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    // Add region requirements

    for(auto idx_space : data.index_spaces()){
      auto& flecsi_ispace = data.index_space_info(idx_space);

      Legion::LogicalPartition color_lpart =
        runtime->get_logical_partition(ctx,
          flecsi_ispace.logical_region, flecsi_ispace.index_partition);
      
      Legion::LogicalRegion color_lregion =
        runtime->get_logical_subregion_by_color(ctx, color_lpart, color);

      Legion::RegionRequirement reg_req(color_lregion, READ_WRITE, SIMULTANEOUS,
        flecsi_ispace.logical_region);

      reg_req.add_field(ghost_owner_pos_fid);

      for(const field_info_t& field_info : context_.registered_fields()){
        if(field_info.index_space == idx_space){
          reg_req.add_field(field_info.fid);
        }
      }

      spmd_launcher.add_region_requirement(reg_req);

      flecsi::coloring::coloring_info_t color_info = 
        coloring_info[idx_space][color];
      
      for(auto ghost_owner : color_info.ghost_owners) {
        clog(trace) << " Color " << color << " idx_space " << idx_space << 
          " has owner " << ghost_owner << std::endl;
        Legion::LogicalRegion ghost_owner_lregion =
          runtime->get_logical_subregion_by_color(ctx, color_lpart, ghost_owner);

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

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, spmd_launcher);
  } // for color

  // Launch the spmd tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  future.wait_all_results();

  // Finish up Legion runtime and fall back out to MPI.

  for (auto& itr : phase_barriers_map) {
    const size_t handle = itr.first;
    for (size_t color = 0; color < phase_barriers_map[handle].size(); color ++) {
      runtime->destroy_phase_barrier(ctx, phase_barriers_map[handle][color]);
    }
    phase_barriers_map[handle].clear();
  }
  phase_barriers_map.clear();

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
