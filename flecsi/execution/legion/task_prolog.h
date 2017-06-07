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

#include <vector>

#include "legion.h"
#include "flecsi/data/data.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/internal_field.h"

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
      auto& flecsi_context = context_t::instance();

      bool read_phase = false;
      bool write_phase = false;
      const int my_color = runtime->find_local_MPI_rank();

      if (GHOST_PERMISSIONS != dno)
        read_phase = true;

      if ( (SHARED_PERMISSIONS == dwd) || (SHARED_PERMISSIONS == drw) )
        write_phase = true;

      if (read_phase) {
        if (!h.ghost_is_readable) {
          clog(error) << "rank " << my_color <<
              " READ PHASE PROLOGUE" << std::endl;
          // as master
          clog(trace) << "rank " << my_color << " arrives & advances " <<
              *(h.pbarrier_as_owner_ptr) <<
              std::endl;

          h.pbarrier_as_owner_ptr->arrive(1);                     // phase WRITE
          *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(context,
              *(h.pbarrier_as_owner_ptr));                          // phase WRITE

          // as slave
          for (size_t owner=0; owner<h.ghost_owners_pbarriers_ptrs.size(); owner++) {
            clog(trace) << "rank " << my_color << " WAITS " <<
                *(h.ghost_owners_pbarriers_ptrs[owner]) <<
                std::endl;

            clog(trace) << "rank " << my_color << " arrives & advances " <<
                *(h.ghost_owners_pbarriers_ptrs[owner]) <<
                std::endl;

            Legion::RegionRequirement rr_shared(h.shared_lr,
              READ_ONLY, EXCLUSIVE, h.color_region);

            Legion::RegionRequirement rr_ghost(h.ghost_lr,
              WRITE_DISCARD, EXCLUSIVE, h.color_region);

            auto iitr = flecsi_context.field_info_map().find(h.index_space);
            clog_assert(iitr != flecsi_context.field_info_map().end(),
              "invalid index space");

            auto ghost_owner_pos_fid = 
              LegionRuntime::HighLevel::FieldID(
              internal_field::ghost_owner_pos);

            rr_ghost.add_field(ghost_owner_pos_fid);

            for(auto& fitr : iitr->second){
              const context_t::field_info_t& fi = fitr.second;
              rr_shared.add_field(fi.fid);
              rr_ghost.add_field(fi.fid);
            }

            // TODO - circular dependency including internal_task.h
            auto constexpr key = 
              flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(
                ghost_copy_task)}.hash();

            const auto ghost_copy_tid = flecsi_context.task_id<key>();
            
            struct {
              size_t index_space;
              size_t owner;
            } args;
            args.index_space = h.index_space;
            args.owner = owner;
            Legion::TaskLauncher
              launcher(ghost_copy_tid,
              Legion::TaskArgument(&args, sizeof(args)));
            
            clog(error) << "SENT SIZE " << h.global_to_local_color_map.size() << std::endl;
            for(auto itr = h.global_to_local_color_map.begin(); itr !=
                    h.global_to_local_color_map.end(); itr++)
                clog(error) << itr->first << " SENT AS " << itr->second << std::endl;

            launcher.add_future(Legion::Future::from_value(runtime,
                    h.global_to_local_color_map));
            launcher.add_region_requirement(rr_shared);
            launcher.add_region_requirement(rr_ghost);
            launcher.add_wait_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));// phase READ
            launcher.add_arrival_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));// phase WRITE
            runtime->execute_task(context, launcher);

            *(h.ghost_owners_pbarriers_ptrs[owner]) = runtime->advance_phase_barrier(context,
                *(h.ghost_owners_pbarriers_ptrs[owner]));             // phase WRITE

          }  // for owner as user

          h.ghost_is_readable = true;
        } // !ghost_is_readable
      } // read_phase

      if (write_phase) {
        clog(error) << "rank " << runtime->find_local_MPI_rank() <<
            " WRITE PHASE PROLOGUE" << std::endl;
        clog(trace) << "rank " << my_color << " wait & arrival barrier " <<
            *(h.pbarrier_as_owner_ptr) <<
            std::endl;
        launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));      // phase WRITE
        launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));   // phase READ
      }
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
