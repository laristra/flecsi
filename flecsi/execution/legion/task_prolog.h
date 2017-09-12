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

          {
          clog_tag_guard(prolog);
          clog(trace) << "rank " << my_color << " WAITS " <<
            *(h.ghost_owners_pbarriers_ptrs[owner]) << std::endl;

          clog(trace) << "rank " << my_color << " arrives & advances " <<
            *(h.ghost_owners_pbarriers_ptrs[owner]) << std::endl;
          } // scope

          Legion::RegionRequirement rr_shared(h.ghost_owners_lregions[owner],
            READ_ONLY, EXCLUSIVE, h.ghost_owners_lregions[owner]);

          Legion::RegionRequirement rr_ghost(h.ghost_lr,
            WRITE_DISCARD, EXCLUSIVE, h.color_region);

          // auto iitr = flecsi_context.field_info_map().find(
          //   { h.data_client_hash, h.index_space });

          // clog_assert(iitr != flecsi_context.field_info_map().end(),
          //   "invalid index space");

          auto ghost_owner_pos_fid = LegionRuntime::HighLevel::FieldID(
            internal_field::ghost_owner_pos);

          rr_ghost.add_field(ghost_owner_pos_fid);

          // for(auto& fitr: iitr->second) {
          //   const context_t::field_info_t & fi = fitr.second;
          //   if(!utils::hash::is_internal(fi.key)){
          //     rr_shared.add_field(fi.fid);
          //     rr_ghost.add_field(fi.fid);
          //   }
          // } // for

          rr_shared.add_field(h.fid);
          rr_ghost.add_field(h.fid);

          // TODO - circular dependency including internal_task.h
          auto constexpr key = flecsi::utils::const_string_t{
            EXPAND_AND_STRINGIFY(ghost_copy_task)}.hash();

          const auto ghost_copy_tid = flecsi_context.task_id<key>();

          // Anonymous struct for arguments in task launcer below.
          struct {
            size_t data_client_hash;
            size_t index_space;
            size_t owner;
          } args;

          args.data_client_hash = h.data_client_hash;
          args.index_space = h.index_space;
          args.owner = owner;

          Legion::TaskLauncher launcher(ghost_copy_tid,
            Legion::TaskArgument(&args, sizeof(args)));

          {
          clog_tag_guard(prolog);
          clog(trace) << "gid to lid map size = " <<
            h.global_to_local_color_map_ptr->size() << std::endl;
          } // scope

          launcher.add_future(Legion::Future::from_value(runtime,
            *(h.global_to_local_color_map_ptr)));

          // Phase READ
          launcher.add_region_requirement(rr_shared);
          launcher.add_region_requirement(rr_ghost);
          launcher.add_wait_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));

          // Phase WRITE
          launcher.add_arrival_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));

          // Execute the ghost copy task
          runtime->execute_task(context, launcher);

          // Phase WRITE
          *(h.ghost_owners_pbarriers_ptrs[owner]) =
            runtime->advance_phase_barrier(context,
            *(h.ghost_owners_pbarriers_ptrs[owner]));
        } // for
      } // read_phase

      if(write_phase) {
        {
        clog_tag_guard(prolog);
        clog(trace) << "rank " << runtime->find_local_MPI_rank() <<
          " WRITE PHASE PROLOGUE" << std::endl;
        clog(trace) << "rank " << my_color << " wait & arrival barrier " <<
          *(h.pbarrier_as_owner_ptr) << std::endl;
        } // scope

        // Phase WRITE
        launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));

        // Phase READ
        launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));
      } // if
      }//end if
    } // handle

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

  }; // struct task_prolog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_prolog_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
