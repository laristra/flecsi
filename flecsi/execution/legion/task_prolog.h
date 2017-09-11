/*~--------------------------------------------------------------------------~*
*  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
* /@@/////  /@@          @@////@@ @@////// /@@
* /@@       /@@  @@@@@  @@    // /@@       /@@
* /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
* /@@////   /@@/@@@@@@@/@@       ////////@@/@@
* /@@       /@@/@@//// //@@    @@       /@@/@@
* /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
* //       ///  //////   //////  ////////  //
*
* Copyright (c) 2016 Los Alamos National Laboratory, LLC
* All rights reserved
*~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_prolog_h
#define flecsi_execution_legion_task_prolog_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <legion.h>
#include <vector>

#include "flecsi/data/data.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/internal_field.h"

clog_register_tag(prolog);

namespace flecsi {
namespace execution {

  //--------------------------------------------------------------------------//
  //! The task_prolog_t type can be called to walk the task args after the
  //! task launcher is created, but before the task has run. This allows
  //! synchronization dependencies to be added to the execution flow.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct task_prolog_t : public utils::tuple_walker__<task_prolog_t>
  {

    //------------------------------------------------------------------------//
    //! Construct a task_prolog_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    task_prolog_t(
      Legion::Runtime * runtime,
      Legion::Context & context,
      Legion::TaskLauncher & launcher
    )
    :
      runtime(runtime),
      context(context),
      launcher(launcher)
    {
    } // task_prolog_t

    //------------------------------------------------------------------------//
    //! FIXME: Need a description.
    //!
    //! @tparam T                     The data type referenced by the handle.
    //! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
    //!                               indices of the index partition.
    //! @tparam SHARED_PERMISSIONS    The permissions required on the shared
    //!                               indices of the index partition.
    //! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
    //!                               indices of the index partition.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS
    >
    void
    handle(
      data_handle__<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & h
    )
    {
 if (!h.global && !h.color){
      auto& flecsi_context = context_t::instance();

      bool read_phase = false;
      bool write_phase = false;
      const int my_color = runtime->find_local_MPI_rank();

      read_phase = GHOST_PERMISSIONS != reserved;
      write_phase = (SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw);

      if(read_phase) {
       if(!*(h.ghost_is_readable)) {
        {
        clog_tag_guard(prolog);
        clog(trace) << "rank " << my_color << " READ PHASE PROLOGUE" <<
          std::endl;

        // As owner
        clog(trace) << "rank " << my_color << " arrives & advances " <<
          *(h.pbarrier_as_owner_ptr) << std::endl;
        } // scope

        // Phase WRITE
        h.pbarrier_as_owner_ptr->arrive(1);

        // Phase WRITE
        *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(context,
          *(h.pbarrier_as_owner_ptr));

        const size_t _pbp_size = h.ghost_owners_pbarriers_ptrs.size();

        // As user
        for(size_t owner{0}; owner<_pbp_size; owner++) {

          owner_regions.push_back(h.ghost_owners_lregions[owner]);
//          Legion::RegionRequirement rr_shared(h.ghost_owners_lregions[owner],
//            READ_ONLY, EXCLUSIVE, h.ghost_owners_lregions[owner]);

          ghost_regions.push_back(h.ghost_lr);
          color_regions.push_back(h.color_region);
//          Legion::RegionRequirement rr_ghost(h.ghost_lr,
//            WRITE_DISCARD, EXCLUSIVE, h.color_region);

//          auto ghost_owner_pos_fid = LegionRuntime::HighLevel::FieldID(
//            internal_field::ghost_owner_pos);

//          rr_ghost.add_field(ghost_owner_pos_fid);

#if 0
          // for(auto& fitr: iitr->second) {
          //   const context_t::field_info_t & fi = fitr.second;
          //   if(!utils::hash::is_internal(fi.key)){
          //     rr_shared.add_field(fi.fid);
          //     rr_ghost.add_field(fi.fid);
          //   }
          // } // for
#endif
          fids.push_back(h.fid);
//          rr_shared.add_field(h.fid);
//          rr_ghost.add_field(h.fid);

          // TODO - circular dependency including internal_task.h
//          auto constexpr key = flecsi::utils::const_string_t{
//            EXPAND_AND_STRINGIFY(ghost_copy_task)}.hash();

//          const auto ghost_copy_tid = flecsi_context.task_id<key>();

          // Anonymous struct for arguments in task launcher below.
          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          local_args.owner = owner;
          args.push_back(local_args);

          //Legion::TaskLauncher ghost_launcher(ghost_copy_tid,
          //  Legion::TaskArgument(&args, sizeof(args)));

          futures.push_back(Legion::Future::from_value(runtime,
              *(h.global_to_local_color_map_ptr)));
//          ghost_launcher.add_future(Legion::Future::from_value(runtime,
//            *(h.global_to_local_color_map_ptr)));

          // Phase READ
          //ghost_launcher.add_region_requirement(rr_shared);
          //ghost_launcher.add_region_requirement(rr_ghost);
          barrier_ptrs.push_back(h.ghost_owners_pbarriers_ptrs[owner]);
//          ghost_launcher.add_wait_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));

          // Phase WRITE
//          ghost_launcher.add_arrival_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));

          // Execute the ghost copy task
//          runtime->execute_task(context, ghost_launcher);

          // Phase WRITE
//          *(h.ghost_owners_pbarriers_ptrs[owner]) =
//            runtime->advance_phase_barrier(context,
//            *(h.ghost_owners_pbarriers_ptrs[owner]));
        } // for owner as user

        *(h.ghost_is_readable) = true;

       } // !ghost_is_readable
      } // read_phase

      if(write_phase && (*h.ghost_is_readable)) {
        // Phase WRITE
        launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));

        // Phase READ
        launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));

        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
      } // if
      }//end if
    } // handle

void launch_copies()
{
  auto& flecsi_context = context_t::instance();

  // As user
  for(size_t owner{0}; owner<owner_regions.size(); owner++) {

    Legion::RegionRequirement rr_shared(owner_regions[owner],
        READ_ONLY, EXCLUSIVE, owner_regions[owner]);

    Legion::RegionRequirement rr_ghost(ghost_regions[owner],
        WRITE_DISCARD, EXCLUSIVE, color_regions[owner]);

    auto ghost_owner_pos_fid = LegionRuntime::HighLevel::FieldID(
        internal_field::ghost_owner_pos);

    rr_ghost.add_field(ghost_owner_pos_fid);

#if 0
    // for(auto& fitr: iitr->second) {
    //   const context_t::field_info_t & fi = fitr.second;
    //   if(!utils::hash::is_internal(fi.key)){
    //     rr_shared.add_field(fi.fid);
    //     rr_ghost.add_field(fi.fid);
    //   }
    // } // for
#endif
    rr_shared.add_field(fids[owner]);
    rr_ghost.add_field(fids[owner]);

    // TODO - circular dependency including internal_task.h
    auto constexpr key = flecsi::utils::const_string_t{
      EXPAND_AND_STRINGIFY(ghost_copy_task)}.hash();

    const auto ghost_copy_tid = flecsi_context.task_id<key>();

    Legion::TaskLauncher ghost_launcher(ghost_copy_tid,
        Legion::TaskArgument(&args[owner], sizeof(args[owner])));

    ghost_launcher.add_future(futures[owner]);

    // Phase READ
    ghost_launcher.add_region_requirement(rr_shared);
    ghost_launcher.add_region_requirement(rr_ghost);
    ghost_launcher.add_wait_barrier(*(barrier_ptrs[owner]));

    // Phase WRITE
    ghost_launcher.add_arrival_barrier(*(barrier_ptrs[owner]));

    // Execute the ghost copy task
    runtime->execute_task(context, ghost_launcher);

    // Phase WRITE
    *(barrier_ptrs[owner]) =
        runtime->advance_phase_barrier(context,
            *(barrier_ptrs[owner]));
  } // for owner as user

}
    //------------------------------------------------------------------------//
    //! FIXME: Need to document.
    //------------------------------------------------------------------------//

    template<
      typename T
    >
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base_t, T>::value>
    handle(
      T&
    )
    {
    } // handle

    Legion::Runtime* runtime;
    Legion::Context & context;
    Legion::TaskLauncher& launcher;
    std::vector<Legion::LogicalRegion> owner_regions;
    std::vector<Legion::LogicalRegion> ghost_regions;
    std::vector<Legion::LogicalRegion> color_regions;
    std::vector<Legion::FieldID> fids;
    struct ghost_copy_args {
      size_t data_client_hash;
      size_t index_space;
      size_t owner;
    };
    std::vector<struct ghost_copy_args> args;
    std::vector<Legion::Future> futures;
    std::vector<Legion::PhaseBarrier*> barrier_ptrs;

  }; // struct task_prolog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_prolog_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
