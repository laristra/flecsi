/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

namespace flecsi {
namespace execution {

/*!
  FIXME
  @ingroup execution
 */
struct finalize_handles_t : public utils::tuple_walker__<finalize_handles_t> {

  /*!
    FIXME
     @ingroup execution
   */

  template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS>
  void handle(dense_accessor__<
              T,
              EXCLUSIVE_PERMISSIONS,
              SHARED_PERMISSIONS,
              GHOST_PERMISSIONS> & a) {
#ifndef MAPPER_COMPACTION

  auto & h = a.handle;

  if ((EXCLUSIVE_PERMISSIONS == rw) || (EXCLUSIVE_PERMISSIONS == wo))
    std::memcpy(h.exclusive_buf, h.exclusive_data, h.exclusive_size * sizeof(T));


  if ((SHARED_PERMISSIONS == rw) || (SHARED_PERMISSIONS == wo))
    std::memcpy(h.shared_buf, h.shared_data, h.shared_size * sizeof(T));

#endif
  } // handle

  /*!
     The finalize_handles_t type can be called to walk task args after task
     execution. This allows us to free memory allocated during the task.
  
     @ingroup execution
   */

  template<typename T, size_t PERMISSIONS>
  void handle(data_client_handle__<T, PERMISSIONS> & h) {
    h.delete_storage();
  } // handle

  /*!
    If this is not a data handle, then simply skip it.
   */

  template<typename T, launch_type_t launch>
  void handle(legion_future__<T, launch>  &h) {
    h.finalize_future();
  }

  template<typename T>
  static typename std::enable_if_t<
      !std::is_base_of<dense_accessor_base_t, T>::value>
  handle(T &) {} // handle

}; // struct finalize_handles_t

} // namespace execution
} // namespace flecsi
