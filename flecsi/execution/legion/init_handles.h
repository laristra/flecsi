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
      const std::vector<Legion::PhysicalRegion>& regions
    )
    :
      runtime(runtime),
      context(context),
      regions(regions),
      region(0)
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
      for(size_t p = 0; p < 4; ++p){
        bool skip = false;

        switch(p){
          case 0:
            skip = EXCLUSIVE_PERMISSIONS == 0 && SHARED_PERMISSIONS == 0;
            break;
          case 1:
            skip = EXCLUSIVE_PERMISSIONS == 0;
            break;
          case 2:
            skip = SHARED_PERMISSIONS == 0;
            break;
          case 3:
            skip = GHOST_PERMISSIONS == 0;
            break;
          default:
            assert(false);
        }

        T* data;
        Legion::PhysicalRegion pr;
        size_t size;

        if(skip){
          data = nullptr;
        }
        else{
          pr = regions[region];
          Legion::LogicalRegion lr = pr.get_logical_region();
          Legion::IndexSpace is = lr.get_index_space();

          auto ac = pr.get_field_accessor(h.fid).template typeify<T>();
          
          Legion::Domain domain = 
            runtime->get_index_space_domain(context, is); 
          
          LegionRuntime::Arrays::Rect<2> r = domain.get_rect<2>();
          LegionRuntime::Arrays::Rect<2> sr;
          LegionRuntime::Accessor::ByteOffset bo[2];
          data = ac.template raw_rect_ptr<2>(r, sr, bo);
          size = r.hi[1] - r.lo[1];
        }

        region++;

        switch(p){
          case 0:
            h.primary_pr = pr;
            h.primary_data = data;
            h.primary_size = size;
            break;
          case 1:
            h.exclusive_pr = pr;
            h.exclusive_data = data;
            h.exclusive_size = size;
            break;
          case 2:
            h.shared_pr = pr;
            h.shared_data = data;
            h.shared_size = size;
            break;
          case 3:
            h.ghost_pr = pr;
            h.ghost_data = data;
            h.shared_size = size;
            break;
          default:
            assert(false);
        }
      }
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
    size_t region;
  }; // struct init_handles_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_init_handles_h
