/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_finalize_handles_h
#define flecsi_execution_mpi_finalize_handles_h

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
