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

#ifndef flecsi_execution_legion_task_epilog_h
#define flecsi_execution_legion_task_epilog_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <legion.h>
#include <type_traits>
#include <vector>

#include "flecsi/utils/tuple_walker.h"

clog_register_tag(epilog);

namespace flecsi {
namespace execution {

  //--------------------------------------------------------------------------//
  //! The task_epilog_t type can be called to walk the task args after the
  //! task has run. This allows synchronization dependencies to be added
  //! to the execution flow.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct task_epilog_t : public utils::tuple_walker__<task_epilog_t>
  {

    //------------------------------------------------------------------------//
    //! Construct a task_epilog_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    task_epilog_t(
      Legion::Runtime * runtime,
      Legion::Context & context
    )
    :
      runtime(runtime),
      context(context)
    {
    } // task_epilog_t

    //------------------------------------------------------------------------//
    //! FIXME: Need description
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
      dense_accessor<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & a
    )
    {
    auto& h = a.handle;

    if (!h.global && !h.color){
      bool write_phase{(SHARED_PERMISSIONS == wo) ||
        (SHARED_PERMISSIONS == rw)};

      if(write_phase && (*h.write_phase_started)) {
        const int my_color = runtime->find_local_MPI_rank();

        {
        clog(trace) << "rank " << my_color << " WRITE PHASE EPILOGUE" <<
          std::endl;

        clog(trace) << "rank " << my_color << " advances " <<
          *(h.pbarrier_as_owner_ptr) <<  std::endl;
        } // scope

        *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(context,

        // Phase READ
        *(h.pbarrier_as_owner_ptr));

        const size_t _pbp_size = h.ghost_owners_pbarriers_ptrs.size();

        // As user
        for(size_t owner=0; owner<_pbp_size; owner++) {
          {
          clog_tag_guard(epilog);
          clog(trace) << "rank " << my_color << " arrives & advances " <<
            *(h.ghost_owners_pbarriers_ptrs[owner]) << std::endl;
          } // scope

          // Phase READ
          h.ghost_owners_pbarriers_ptrs[owner]->arrive(1);
          *(h.ghost_owners_pbarriers_ptrs[owner]) =
            runtime->advance_phase_barrier(context,

          // Phase READ
          *(h.ghost_owners_pbarriers_ptrs)[owner]);
        } // for
        *(h.write_phase_started) = false;
        }//if write phase

      } // if global and color
    } // handle

    //------------------------------------------------------------------------//
    //! FIXME: Need to document.
    //!
    //! @param T
    //------------------------------------------------------------------------//

    template<
      typename T
    >
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base_t, T>::value>
    handle(
      T &
    )
    {
    } // handle

    Legion::Runtime * runtime;
    Legion::Context & context;

  }; // struct task_epilog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_epilog_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
