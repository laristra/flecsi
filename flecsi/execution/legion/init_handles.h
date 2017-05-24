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

#ifndef flecsi_execution_legion_init_handles_h
#define flecsi_execution_legion_init_handles_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 24, 2017
//----------------------------------------------------------------------------//

#include <vector>

#include "legion.h"

#include "flecsi/data/common/privilege.h"
#include "flecsi/utils/tuple_walker.h"

namespace flecsi {
namespace execution {

  //----------------------------------------------------------------------------//
  //! The init_handles_t type can be called to walk task args after task 
  //! launch. This allows us to map physical regions to internal handle
  //! buffers/accessors.
  //!
  //! @ingroup execution
  //----------------------------------------------------------------------------//

  struct init_handles_t : public utils::tuple_walker__<init_handles_t>
  {

    //--------------------------------------------------------------------------//
    //! Construct an init_handles_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //--------------------------------------------------------------------------//

    init_handles_t(
      Legion::Runtime* runtime,
      Legion::Context& context,
      const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions
    )
    :
      runtime(runtime),
      context(context),
      regions(regions)
    {
    } // init_handles

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

    //-----------------------------------------------------------------------//
    // If this is not a data handle, then simply skip it.
    //-----------------------------------------------------------------------//

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
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions;
  }; // struct init_handles_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_init_handles_h
