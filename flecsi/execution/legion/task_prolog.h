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
      Legion::TaskLauncher & launcher)
      : runtime(runtime), context(context), launcher(launcher) {
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
    if(sparse){
      return;
    }

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

            // As owner
            clog(trace) << "rank " << my_color << " arrives & advances "
                        << *(h.pbarrier_as_owner_ptr) << std::endl;
          } // scope

          // Phase WRITE
          h.pbarrier_as_owner_ptr->arrive(1);

          // Phase WRITE
          *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(
              context, *(h.pbarrier_as_owner_ptr));

          const size_t _pbp_size = h.ghost_owners_pbarriers_ptrs.size();

          // As user
          for (size_t owner{0}; owner < _pbp_size; owner++) {
            owner_regions.push_back(h.ghost_owners_lregions[owner]);
            // ndm - need sparse
            owner_subregions.push_back(h.ghost_owners_subregions[owner]);
            ghost_regions.push_back(h.ghost_lr);
            color_regions.push_back(h.color_region);
            // ndm - need offset and range
            fids.push_back(h.fid);
            ghost_copy_args local_args;
            local_args.data_client_hash = h.data_client_hash;
            local_args.index_space = h.index_space;
            local_args.owner = owner;
            args.push_back(local_args);
            futures.push_back(Legion::Future::from_value(
                runtime, *(h.global_to_local_color_map_ptr)));
            barrier_ptrs.push_back(h.ghost_owners_pbarriers_ptrs[owner]);
          } // for owner as user

          *(h.ghost_is_readable) = true;

        } // !ghost_is_readable
      } // read_phase

      if (write_phase && (*h.ghost_is_readable)) {
        // Phase WRITE
        launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));

        // Phase READ
        launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));

        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
      } // if
    } // end if
  } // handle

  /*!
   Walk the data handles for a flecsi task, store info for ghost copies
   in member variables, and add phase barriers to launcher as needed.

   Use member variables initialized by the walk to launch 1 copy per owner
   region

   */

  void launch_copies() {
    auto & flecsi_context = context_t::instance();

    // group owners by owner_regions
    std::vector<std::set<size_t>> owner_groups;
    std::vector<bool> is_sparse_group;
    for (size_t owner{0}; owner < owner_regions.size(); owner++) {
      bool found_group = false;
      for (size_t group{0}; group < owner_groups.size(); group++) {
        auto first = owner_groups[group].begin();
        if (owner_regions[owner] == owner_regions[*first]) {
          owner_groups[group].insert(owner);
          found_group = true;
          continue;
        }
      } // for group
      if (!found_group) {
        std::set<size_t> new_group;
        new_group.insert(owner);
        owner_groups.push_back(new_group);

        is_sparse_group.push_back(args[owner].sparse);
      }
    } // for owner

    // launch copy task per group of owners with same owner_region
    for (size_t group{0}; group < owner_groups.size(); group++) {
      bool sparse = is_sparse_group[group];

      auto first_itr = owner_groups[group].begin();
      size_t first = *first_itr;

      Legion::RegionRequirement rr_shared(
          owner_subregions[first], READ_ONLY, EXCLUSIVE, owner_regions[first]);

      Legion::RegionRequirement rr_ghost(
          ghost_regions[first], WRITE_DISCARD, EXCLUSIVE, color_regions[first]);


      Legion::RegionRequirement rr_entries_shared;

      Legion::RegionRequirement rr_entries_ghost;

      if(sparse){
        rr_entries_shared =
          Legion::RegionRequirement(
          owner_entries_regions[first], READ_ONLY, EXCLUSIVE,
          owner_entries_regions[first]);

        rr_entries_ghost =
          Legion::RegionRequirement(
          color_entries_regions[first], WRITE_DISCARD, EXCLUSIVE,
          color_entries_regions[first]);        
      }

      auto ghost_owner_pos_fid =
          LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

      rr_ghost.add_field(ghost_owner_pos_fid);

      // TODO - circular dependency including internal_task.h
      auto constexpr key =
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(ghost_copy_task)}
              .hash();

      const auto ghost_copy_tid = flecsi_context.task_id<key>();

      // ndm - diff task

      Legion::TaskLauncher ghost_launcher(
          ghost_copy_tid,
          Legion::TaskArgument(&args[first], sizeof(args[first])));

      ghost_launcher.add_future(futures[first]);

      for (auto owner_itr = owner_groups[group].begin();
           owner_itr != owner_groups[group].end(); owner_itr++) {
        size_t owner = *owner_itr;

        rr_shared.add_field(fids[owner]);
        rr_ghost.add_field(fids[owner]);

        if(sparse){
          rr_entries_shared.add_field(fids[owner]);
          rr_entries_ghost.add_field(fids[owner]);
        }

        // Phase READ
        ghost_launcher.add_wait_barrier(*(barrier_ptrs[owner]));

        // Phase WRITE
        ghost_launcher.add_arrival_barrier(*(barrier_ptrs[owner]));

        // Phase WRITE
        *(barrier_ptrs[owner]) =
            runtime->advance_phase_barrier(context, *(barrier_ptrs[owner]));
      } // for owner
      // ndm - add rr for sparse shared & ghost
      ghost_launcher.add_region_requirement(rr_shared);
      ghost_launcher.add_region_requirement(rr_ghost);

      if(sparse){
        ghost_launcher.add_region_requirement(rr_entries_shared);
        ghost_launcher.add_region_requirement(rr_entries_ghost);
      }

      // Execute the ghost copy task
      runtime->execute_task(context, ghost_launcher);

    } // for group

  } // launch copies

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    sparse_accessor <
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS
    > &a
  )
  {
    if(!sparse){
      return;
    }

    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = a.handle;

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

          // As owner
          clog(trace) << "rank " << my_color << " arrives & advances "
                      << *(h.pbarrier_as_owner_ptr) << std::endl;
        } // scope

        // Phase WRITE
        h.pbarrier_as_owner_ptr->arrive(1);

        // Phase WRITE
        *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(
            context, *(h.pbarrier_as_owner_ptr));

        const size_t _pbp_size = h.ghost_owners_pbarriers_ptrs.size();

        // As user
        for (size_t owner{0}; owner < _pbp_size; owner++) {

          // offsets
          owner_regions.push_back(h.ghost_owners_offsets_lregions[owner]);
          owner_subregions.push_back(h.ghost_owners_offsets_subregions[owner]);

          owner_entries_regions.push_back(
            h.ghost_owners_entries_lregions[owner]);
          /*
          owner_entries_subregions.push_back(
            h.ghost_owners_entries_subregions[owner]);
            */

          ghost_regions.push_back(h.offsets_ghost_lr);
          ghost_entries_regions.push_back(h.entries_ghost_lr);
          
          color_regions.push_back(h.offsets_color_region);
          color_entries_regions.push_back(h.entries_color_region);

          fids.push_back(h.fid);

          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          local_args.owner = owner;
          local_args.sparse = true;
          local_args.reserve = h.reserve;
          local_args.max_entries_per_index = h.max_entries_per_index;
          args.push_back(local_args);

          futures.push_back(Legion::Future::from_value(
              runtime, *(h.global_to_local_color_map_ptr)));
          barrier_ptrs.push_back(h.ghost_owners_pbarriers_ptrs[owner]);
        } // for owner as user

        *(h.ghost_is_readable) = true;

      } // !ghost_is_readable
    } // read_phase

    if (write_phase && (*h.ghost_is_readable)) {
      // Phase WRITE
      launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));

      // Phase READ
      launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));

      *(h.ghost_is_readable) = false;
      *(h.write_phase_started) = true;
    } // if
  }

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void
  handle(
    ragged_accessor<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & a
  )
  {
    handle(reinterpret_cast<sparse_accessor<
      T, EXCLUSIVE_PERMISSIONS, SHARED_PERMISSIONS, GHOST_PERMISSIONS>&>(a));
  } // handle

  template<
    typename T
  >
  void
  handle(
    sparse_mutator<
    T
    > &m
  )
  {
    if(!sparse){
      return;
    }
    
    auto & h = m.h_;

    if ((*h.ghost_is_readable)) {
      // Phase WRITE
      launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));

      // Phase READ
      launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));

      *(h.ghost_is_readable) = false;
      *(h.write_phase_started) = true;
    } // if
  }

  template<
    typename T
  >
  void
  handle(
    ragged_mutator<
      T
    > & m
  )
  {
    handle(reinterpret_cast<sparse_mutator<T>&>(m));
  }

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
  Legion::TaskLauncher & launcher;
  std::vector<Legion::LogicalRegion> owner_regions;
  std::vector<Legion::LogicalRegion> owner_subregions;
  std::vector<Legion::LogicalRegion> owner_entries_regions;
  //std::vector<Legion::LogicalRegion> owner_entries_subregions;
  std::vector<Legion::LogicalRegion> ghost_regions;
  std::vector<Legion::LogicalRegion> color_regions;
  std::vector<Legion::LogicalRegion> ghost_entries_regions;
  std::vector<Legion::LogicalRegion> color_entries_regions;
  std::vector<Legion::FieldID> fids;
  struct ghost_copy_args {
    size_t data_client_hash;
    size_t index_space;
    size_t owner;
    bool sparse = false;
    size_t reserve;
    size_t max_entries_per_index;
  };
  std::vector<struct ghost_copy_args> args;
  std::vector<Legion::Future> futures;
  std::vector<Legion::PhaseBarrier *> barrier_ptrs;
  size_t reserve;
  size_t max_entries_per_index;
  bool sparse = false;

}; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
