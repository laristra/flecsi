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

#include <vector>
#include <type_traits>

#include "legion.h"

#include "flecsi/utils/tuple_walker.h"

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
      data_handle__<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & h
    )
    {
      bool write_phase = false;

      if ( (SHARED_PERMISSIONS == dwd) || (SHARED_PERMISSIONS == drw) )
        write_phase = true;

      if (write_phase) {
        const int my_color = runtime->find_local_MPI_rank();
        clog(error) << "rank " << my_color << " WRITE PHASE EPILOGUE" << std::endl;

        clog(trace) << "rank " << my_color << " advances " << *(h.pbarrier_as_owner_ptr) <<
            std::endl;
        *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(context,
            *(h.pbarrier_as_owner_ptr));             // phase READ

        // as slave
        for (size_t owner=0; owner<h.ghost_owners_pbarriers_ptrs.size(); owner++) {
          clog(trace) << "rank " << my_color << " arrives & advances " <<
              *(h.ghost_owners_pbarriers_ptrs[owner]) <<
              std::endl;

          h.ghost_owners_pbarriers_ptrs[owner]->arrive(1);  // phase READ
          *(h.ghost_owners_pbarriers_ptrs[owner]) = runtime->advance_phase_barrier(context,
              *(h.ghost_owners_pbarriers_ptrs[owner]));       // phase READ

        }

        h.ghost_is_readable = false;
      } // write_phase
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
    handle(T &)
    {
    } // handle

    Legion::Runtime* runtime;
    Legion::Context & context;

  }; // struct task_epilog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_epilog_h
