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

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/legion_tasks.h"
#include "flecsi/execution/legion/mapper.h"
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

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization driver task" << std::endl;
  }

  // Set the current task context to the driver
  context_.push_state(utils::const_string_t{"specialization_driver"}.hash(),
    ctx, runtime, task, regions);

  // run default or user-defined specialization driver 
  specialization_driver(args.argc, args.argv);

  // Set the current task context to the driver
  context_.pop_state( utils::const_string_t{"specialization_driver"}.hash());
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI num_colors is " << num_colors << std::endl;
  }

#if 0
for(auto p: context_.partitions()) {
  clog_container_one(info, "exclusive", p.second.exclusive, clog::space);
} // for
#endif

  std::map<size_t, Legion::IndexSpace> expanded_ispaces_map;
  std::map<size_t, Legion::FieldSpace> expanded_fspaces_map;
  std::map<size_t, Legion::LogicalRegion> expanded_lregions_map;
  std::map<size_t, Legion::IndexPartition> color_iparts_map;
  std::map<size_t, std::vector<Legion::PhaseBarrier>> phase_barriers_map;

  LegionRuntime::Arrays::Rect<1> color_bounds(0, num_colors - 1);
  Legion::Domain color_domain = Legion::Domain::from_rect<1>(color_bounds);

  {
  clog_tag_guard(runtime_driver);
  auto coloring_info = context_.coloring_info_map();

  for(auto key_idx: coloring_info) {
    // Create expanded IndexSpace
    clog(error) << "index: " << key_idx.first << std::endl;
    size_t total_num_entities = 0;
    for(auto color_idx: key_idx.second) {
      clog(error) << "color: " << color_idx.first << " " << color_idx.second << std::endl;
      total_num_entities = std::max(total_num_entities,
          color_idx.second.exclusive + color_idx.second.shared + color_idx.second.ghost);
    } // for color_idx
    clog(error) << "total_num_entities " << total_num_entities << std::endl;

    LegionRuntime::Arrays::Rect<2> expanded_bounds = LegionRuntime::Arrays::Rect<2>(
        LegionRuntime::Arrays::Point<2>::ZEROES(),
        LegionRuntime::Arrays::make_point(num_colors,total_num_entities));
    Legion::Domain expanded_dom(Legion::Domain::from_rect<2>(expanded_bounds));
    Legion::IndexSpace expanded_is = runtime->create_index_space(ctx, expanded_dom);
    char buf[80];
    sprintf(buf, "expanded index space %ld", key_idx.first);
    runtime->attach_name(expanded_is, buf);
    expanded_ispaces_map[key_idx.first] = expanded_is;

    // Read user + FleCSI registered field spaces
    Legion::FieldSpace expanded_fs = runtime->create_field_space(ctx);
    {
      Legion::FieldAllocator allocator = runtime->create_field_allocator(ctx, expanded_fs);
      allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<2>), 42); // FIXME use registration
    }
    sprintf(buf, "expanded field space %ld", key_idx.first);
    runtime->attach_name(expanded_fs, buf);
    expanded_fspaces_map[key_idx.first] = expanded_fs;

    Legion::LogicalRegion expanded_lr = runtime->create_logical_region(ctx,
        expanded_is, expanded_fs);
    sprintf(buf, "expanded logical region %ld", key_idx.first);
    runtime->attach_name(expanded_lr, buf);
    expanded_lregions_map[key_idx.first] = expanded_lr;

    // Partition expanded IndexSpace color-wise & create associated PhaseBarriers
    Legion::DomainColoring color_partitioning;
    for (int color = 0; color < num_colors; color++) {
      auto color_idx = key_idx.second.find(color);
      clog_assert(color_idx != key_idx.second.end(),
        "color missing in ColorIndexSpace");
      LegionRuntime::Arrays::Rect<2> subrect(
          LegionRuntime::Arrays::make_point(color, 0),
          LegionRuntime::Arrays::make_point(color,
            color_idx->second.exclusive + color_idx->second.shared + color_idx->second.ghost - 1));
      color_partitioning[color] = Legion::Domain::from_rect<2>(subrect);
      phase_barriers_map[key_idx.first].push_back(runtime->create_phase_barrier(ctx,
        1 + color_idx->second.shared_users.size()));
      clog(error) << "phase barrier " << color_idx->first << " has " <<
          color_idx->second.shared_users.size() + 1 << " arrivers" << std::endl;
    }

    Legion::IndexPartition color_ip = runtime->create_index_partition(ctx,
        expanded_is, color_domain, color_partitioning, true /*disjoint*/);
    sprintf(buf, "color partitioing %ld", key_idx.first);
    runtime->attach_name(color_ip, buf);
    color_iparts_map[key_idx.first] = color_ip;
  } // for key_idx
  } // clog_tag_guard

  // Register user data
  //data::storage_t::instance().register_all();

  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;

  #if 0
  auto spmd_id = context_.task_id(__flecsi_task_key(spmd_task, loc));

  // Add colors to must_epoch_launcher
  for(size_t i(0); i<num_colors; ++i) {
    Legion::TaskLauncher spmd_launcher(spmd_id, Legion::TaskArgument(0, 0));
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    Legion::DomainPoint point(i);
    must_epoch_launcher.add_single_task(point, spmd_launcher);
  } // for

  // Launch the spmd tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  future.wait_all_results();
  #endif

  // Finish up Legion runtime and fall back out to MPI.

  for (auto itr = color_iparts_map.begin(); itr != color_iparts_map.end(); ++itr)
    runtime->destroy_index_partition(ctx, color_iparts_map[itr->first]);
  color_iparts_map.clear();

  for (auto itr = expanded_ispaces_map.begin(); itr != expanded_ispaces_map.end(); ++itr)
    runtime->destroy_index_space(ctx, expanded_ispaces_map[itr->first]);
  expanded_ispaces_map.clear();

  for (auto itr = expanded_fspaces_map.begin(); itr != expanded_fspaces_map.end(); ++itr)
    runtime->destroy_field_space(ctx, expanded_fspaces_map[itr->first]);
  expanded_fspaces_map.clear();

  for (auto itr = expanded_lregions_map.begin(); itr != expanded_lregions_map.end(); ++itr)
    runtime->destroy_logical_region(ctx, expanded_lregions_map[itr->first]);
  expanded_lregions_map.clear();

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
