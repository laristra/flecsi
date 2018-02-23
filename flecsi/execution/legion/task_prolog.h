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
#pragma once

/*! @file */

#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/data/data.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/internal_field.h>

clog_register_tag(prolog);

namespace flecsi {
namespace execution {

/*!
 The task_prolog_t type can be called to walk the task args after the
 task launcher is created, but before the task has run. This allows
 synchronization dependencies to be added to the execution flow.

 @ingroup execution
 */

struct task_prolog_t : public utils::tuple_walker__<task_prolog_t> {

  /*!
   Construct a task_prolog_t instance.
  
   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  task_prolog_t(
      Legion::Runtime * runtime,
      Legion::Context & context,
      Legion::Domain & color_domain)
      : runtime(runtime), context(context), color_domain(color_domain) {
  } // task_prolog_t

  /*!
   Walk the data handles for a flecsi task, store info for ghost copies
   in member variables, and add phase barriers to launcher as needed.
  
   @tparam T                     The data type referenced by the handle.
   @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                 indices of the index partition.
   @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                 indices of the index partition.
   @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                 indices of the index partition.
  
   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS>
  void handle(dense_accessor__<
              T,
              EXCLUSIVE_PERMISSIONS,
              SHARED_PERMISSIONS,
              GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    if (!h.global && !h.color) {
      auto & flecsi_context = context_t::instance();

      bool read_phase = false;
      bool write_phase = false;
      const int my_color = runtime->find_local_MPI_rank();

      read_phase = GHOST_PERMISSIONS != reserved;
      write_phase = (SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw);

      if (read_phase) {
        if (!*(h.ghost_is_readable)) {
          {
            clog_tag_guard(prolog);
            clog(trace) << "rank " << my_color << " READ PHASE PROLOGUE"
                        << std::endl;
          } // scope

          // As user

          ghost_owners_partitions.push_back(h.ghost_owners_lp);
          ghost_partitions.push_back(h.ghost_lp);
          entire_regions.push_back(h.entire_region);
          fids.push_back(h.fid);
          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          args.push_back(local_args);

          *(h.ghost_is_readable) = true;

        } // !ghost_is_readable
      } // read_phase

      if (write_phase && (*h.ghost_is_readable)) {
        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
      } // if
    } // end if

  } // handle

  /*!
   Use member variables initialized by the walk to launch 1 copy per owner
   region
  
   */

  void launch_copies() {
    auto & flecsi_context = context_t::instance();

    // group by ghost_owners_partition
    std::vector<std::set<size_t>> handle_groups;
    for (size_t handle{0}; handle < ghost_owners_partitions.size(); handle++) {
      bool found_group = false;
      for (size_t group{0}; group < handle_groups.size(); group++) {
        auto first = handle_groups[group].begin();
        if (ghost_owners_partitions[handle] == ghost_owners_partitions[*first]) {
          handle_groups[group].insert(handle);
          found_group = true;
          continue;
        }
      } // for group
      if (!found_group) {
        std::set<size_t> new_group;
        new_group.insert(handle);
        handle_groups.push_back(new_group);
      }
    }  // for handle

    // launch copy task per group of handles with same ghost_owners_partition
    for (size_t group{0}; group < handle_groups.size(); group++) {
      auto first_itr = handle_groups[group].begin();
      size_t first = *first_itr;

      Legion::RegionRequirement rr_owners(ghost_owners_partitions[first],
          0/*projection ID*/, READ_ONLY, EXCLUSIVE, entire_regions[first]);
      Legion::RegionRequirement rr_ghost(ghost_partitions[first],
          0/*projection ID*/, WRITE_DISCARD, EXCLUSIVE, entire_regions[first]);

      auto ghost_owner_pos_fid =
          LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

      rr_ghost.add_field(ghost_owner_pos_fid);

      // TODO - circular dependency including internal_task.h
      auto constexpr key =
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(ghost_copy_task)}
              .hash();

      const auto ghost_copy_tid = flecsi_context.task_id<key>();

      Legion::IndexLauncher ghost_launcher(ghost_copy_tid, color_domain,
          Legion::TaskArgument(&args[first], sizeof(args[first])),
          Legion::ArgumentMap());

      for (auto handle_itr = handle_groups[group].begin();
          handle_itr != handle_groups[group].end(); handle_itr++) {
        size_t handle = *handle_itr;

        rr_owners.add_field(fids[handle]);
        rr_ghost.add_field(fids[handle]);
      }

      ghost_launcher.add_region_requirement(rr_owners);
      ghost_launcher.add_region_requirement(rr_ghost);
      // Execute the ghost copy task
      runtime->execute_index_space(context, ghost_launcher);
    } // for group

  } // launch copies

  /*!  
    Don't do anything with flecsi task argument that are not data handles.
   */

  template<typename T>
  static typename std::enable_if_t<
      !std::is_base_of<dense_accessor_base_t, T>::value>
  handle(T &) {} // handle

  // member variables
  Legion::Runtime * runtime;
  Legion::Context & context;
  Legion::Domain &color_domain;
  std::vector<Legion::LogicalPartition> ghost_owners_partitions;
  std::vector<Legion::LogicalPartition> ghost_partitions;
  std::vector<Legion::LogicalRegion> entire_regions;
  std::vector<Legion::FieldID> fids;
  struct ghost_copy_args {
    size_t data_client_hash;
    size_t index_space;
  };
  std::vector<struct ghost_copy_args> args;

}; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
