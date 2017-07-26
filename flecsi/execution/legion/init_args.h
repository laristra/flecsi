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

#include "flecsi/data/common/privilege.h"
#include "flecsi/data/data_client_handle.h"

namespace flecsi {
namespace execution {

  //--------------------------------------------------------------------------//
  //! The init_args_t type can be called to walk task args before the
  //! task launcher is created. This allows us to gather region requirements
  //! and to set state on the associated data handles \em before Legion gets
  //! the task arguments tuple.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct init_args_t : public utils::tuple_walker__<init_args_t>
  {

    //------------------------------------------------------------------------//
    //! Construct an init_args_t instance.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    init_args_t(
      Legion::Runtime* runtime,
      Legion::Context & context
    )
    :
      runtime(runtime),
      context(context)
    {
    } // init_args

    //------------------------------------------------------------------------//
    //! Convert the template privileges to proper Legion privileges.
    //!
    //! @param mode privilege
    //------------------------------------------------------------------------//

    static Legion::PrivilegeMode
    privilege_mode(
      size_t mode
    ){
      switch(mode){
        case size_t(dno):
          return NO_ACCESS;
        case size_t(dro):
          return READ_ONLY;
        case size_t(dwd):
          return WRITE_DISCARD;
        case size_t(drw):
          return READ_WRITE;
        default:
          assert(false);
      }
    }

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

      Legion::MappingTagID tag = EXCLUSIVE_LR;

      Legion::RegionRequirement ex_rr(h.exclusive_lr,
        privilege_mode(EXCLUSIVE_PERMISSIONS), EXCLUSIVE, h.color_region, tag);
      ex_rr.add_field(h.fid);
      region_reqs.push_back(ex_rr);

      Legion::RegionRequirement sh_rr(h.shared_lr,
        privilege_mode(SHARED_PERMISSIONS), EXCLUSIVE, h.color_region);
      sh_rr.add_field(h.fid);
      region_reqs.push_back(sh_rr);

      Legion::RegionRequirement gh_rr(h.ghost_lr,
        privilege_mode(GHOST_PERMISSIONS), EXCLUSIVE, h.color_region);
      gh_rr.add_field(h.fid);
      region_reqs.push_back(gh_rr);
    } // handle

    template<
      typename T,
      size_t PERMISSIONS
    >
    void
    handle(
      data_client_handle__<T, PERMISSIONS> & h
    )
    {
      auto& context_ = context_t::instance();

      std::unordered_map<size_t, size_t> region_map;

      for(size_t i{0}; i<h.num_handle_entities; ++i) {
        data_client_handle_entity & ent = h.handle_entities[i];

        const size_t index_space = ent.index_space;
        const size_t dim = ent.dim;
        const size_t domain = ent.domain;

        region_map[index_space] = region_reqs.size();

        Legion::RegionRequirement rr(ent.color_region,
          privilege_mode(PERMISSIONS), EXCLUSIVE, ent.color_region);

        rr.add_field(ent.fid);
        region_reqs.push_back(rr);
      } // for

      for(size_t i{0}; i < h.num_handle_adjacencies; ++i){
        data_client_handle_adjacency& adj = h.handle_adjacencies[i];

        const size_t adj_index_space = adj.adj_index_space;
        const size_t from_index_space = adj.from_index_space;
        const size_t to_index_space = adj.to_index_space;

        region_reqs[region_map[from_index_space]].add_field(adj.offset_fid);

        Legion::RegionRequirement adj_rr(adj.adj_region,
          privilege_mode(PERMISSIONS), EXCLUSIVE, adj.adj_region);

        adj_rr.add_field(adj.index_fid);

        region_reqs.push_back(adj_rr);
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
    std::vector<Legion::RegionRequirement> region_reqs;

  }; // struct init_args_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_init_args_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
