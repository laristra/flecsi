/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_finalize_handles_h
#define flecsi_topology_finalize_handles_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 19, 2017
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

struct finalize_handles_t : public utils::tuple_walker__<finalize_handles_t>
{
  //--------------------------------------------------------------------------//
  //! @ingroup execution
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
  //! The finalize_handles_t type can be called to walk task args after task
  //! execution. This allows us to free memory allocated during the task.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t PERMISSIONS
  >
  void
  handle(
    data_client_handle__<T, PERMISSIONS> & h
  )
  {
    // Write changes back to Legion data.
    if(PERMISSIONS == drw || PERMISSIONS == dwd) {
      for(size_t i = 0; i < h.num_handle_adjacencies; ++i) {
        data_client_handle_adjacency_t & adj = h.handle_adjacencies[i];

        auto & conn = h.get_connectivity(adj.from_domain, adj.to_domain,
          adj.from_dim, adj.to_dim, all);
        auto & from_index_vec = conn.get_from_index_vec();

        // We are storing adjacency information for the mesh in CRS
        // format, so we need to convert it back to Legion's storage
        // scheme.
        for(size_t j{0}; j<from_index_vec.size()-1; ++j) {
          adj.offsets_buf[j].x[0] = from_index_vec[j];
          adj.offsets_buf[j].x[1] = from_index_vec[j+1] - from_index_vec[j];
        } // for

        // Write back indices
        auto & ents = conn.get_entities();
        clog_assert(
          ents.size() == adj.num_indices, 
          "Size mismatch in adjacency from (" << adj.from_domain << ", " << 
          adj.from_dim << ") to (" << adj.to_domain << ", " << adj.to_dim <<
          ").  Expected " << adj.num_indices << ", got " <<
          ents.size()
        );
        for(size_t j{0}; j<adj.num_indices; ++j) {
          adj.indices_buf[j] = ents[j].entity();
        } // for
      } // for
    } // if

    h.delete_storage();

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

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_topology_finalize_handles_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
