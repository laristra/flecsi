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

#ifndef flecsi_execution_legion_init_args_h
#define flecsi_execution_legion_init_args_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <vector>

#include "legion.h"

namespace flecsi {
namespace execution {

  //----------------------------------------------------------------------------//
  //! The init_args_t type can be called to walk task args before the
  //! task launcher is created. This allows us to gather region requirements
  //! and to set state on the associated data handles \em before Legion gets
  //! the task arguments tuple.
  //!
  //! @ingroup execution
  //----------------------------------------------------------------------------//

  struct init_args_t : public utils::tuple_walker__<init_args_t>
  {

    //--------------------------------------------------------------------------//
    //! Construct an init_args_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //--------------------------------------------------------------------------//

    init_args_t(
      Legion::Runtime* runtime,
      Legion::Context & context
    )
    :
      runtime(runtime),
      context(context)
    {
    } // init_args

    //--------------------------------------------------------------------------//
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
    //! @param h The data handle.
    //--------------------------------------------------------------------------//

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
    } // handle

    //--------------------------------------------------------------------------//
    //! FIXME: Need to document.
    //--------------------------------------------------------------------------//

    template<
      typename T
    >
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::value>
    handle(
      T &
    )
    {
    } // handle

    Legion::Runtime * runtime;
    Legion::Context & context;
    std::vector<Legion::RegionRequirement> reqs;

  }; // struct init_args_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_init_args_h
