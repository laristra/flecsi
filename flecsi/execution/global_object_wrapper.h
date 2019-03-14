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

#if !defined(__FLECSI_PRIVATE__)
  #error Do not include this file directly!
#endif

#include <cstdint>
#include <type_traits>

namespace flecsi {
namespace execution {

/*!
  The global_object_wrapper_u type provides a mechanism to recover type
  information so that global objects can be properly deleted.
 */

template<typename OBJECT_TYPE>
struct global_object_wrapper_u {

  /*!
    Delete the object referenced by \em address.

    @param address The address of the object to delete.
   */

  static void cleanup(uintptr_t address) {
    if constexpr(std::is_array_v<OBJECT_TYPE>) {
      delete[] reinterpret_cast<OBJECT_TYPE *>(address);
    }
    else {
      delete reinterpret_cast<OBJECT_TYPE *>(address);
    }
  } // cleanup

}; // class global_object_wrapper_u

} // namespace execution
} // namespace flecsi
