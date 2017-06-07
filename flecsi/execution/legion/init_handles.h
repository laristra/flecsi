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

  //--------------------------------------------------------------------------//
  //! The init_handles_t type can be called to walk task args after task 
  //! launch. This allows us to map physical regions to internal handle
  //! buffers/accessors.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct init_handles_t : public utils::tuple_walker__<init_handles_t>
  {

    //------------------------------------------------------------------------//
    //! Construct an init_handles_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    init_handles_t(
      Legion::Runtime* runtime,
      Legion::Context& context,
      const std::vector<Legion::PhysicalRegion>& regions
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

      constexpr size_t num_regions = 3;

      h.context = context;
      h.runtime = runtime;

      Legion::PhysicalRegion prs[num_regions];
      T* data[num_regions];
      size_t sizes[num_regions];
      h.combined_size = 0;
      size_t permissions[] = 
        {EXCLUSIVE_PERMISSIONS, SHARED_PERMISSIONS, GHOST_PERMISSIONS};

      // Get sizes, physical regions, and raw rect buffer for each of ex/sh/gh
      for(size_t r = 0; r < num_regions; ++r){
        if(permissions[r] == 0){
          data[r] = nullptr;
          sizes[r] = 0;
          prs[r] = Legion::PhysicalRegion();
        }
        else{
          prs[r] = regions[r];
          Legion::LogicalRegion lr = prs[r].get_logical_region();
          Legion::IndexSpace is = lr.get_index_space();

          auto ac = prs[r].get_field_accessor(h.fid).template typeify<T>();
          
          Legion::Domain domain = 
            runtime->get_index_space_domain(context, is); 
          
          LegionRuntime::Arrays::Rect<2> dr = domain.get_rect<2>();
          LegionRuntime::Arrays::Rect<2> sr;
          LegionRuntime::Accessor::ByteOffset bo[2];
          data[r] = ac.template raw_rect_ptr<2>(dr, sr, bo);
          data[r] += bo[1];
          sizes[r] = sr.hi[1] - sr.lo[1] + 1;
          h.combined_size += sizes[r];
        }
      }

      // Create the concatenated buffer E+S+G
      h.combined_data = new T[h.combined_size];

      // Set additional fields needed by the data handle/accessor
      // and copy into the combined buffer. Note that exclusive_data, etc.
      // aliases the combined buffer for its respective region.
      size_t pos = 0;
      for(size_t r = 0; r < num_regions; ++r){
        switch(r){
          case 0:
            h.exclusive_size = sizes[r];
            h.exclusive_pr = prs[r];
            h.exclusive_data = h.exclusive_size == 0 ? 
              nullptr : h.combined_data + pos;
            h.exclusive_buf = data[r];
            h.exclusive_priv = EXCLUSIVE_PERMISSIONS;
            break;
          case 1:
            h.shared_size = sizes[r];
            h.shared_pr = prs[r];
            h.shared_data = h.shared_size == 0 ? 
              nullptr : h.combined_data + pos;
            h.shared_buf = data[r];
            h.shared_priv = SHARED_PERMISSIONS;
            break;
          case 2:
            h.ghost_size = sizes[r];
            h.ghost_pr = prs[r];
            h.ghost_data = h.ghost_size == 0 ? 
              nullptr : h.combined_data + pos;
            h.ghost_buf = data[r];
            h.ghost_priv = GHOST_PERMISSIONS;
            break;
          default:
            assert(false);
        }
        
        std::memcpy(h.combined_data + pos, data[r], sizes[r] * sizeof(T));
        pos += sizes[r];
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
  }; // struct init_handles_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_init_handles_h
