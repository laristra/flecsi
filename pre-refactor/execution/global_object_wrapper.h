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
    delete reinterpret_cast<OBJECT_TYPE *>(address);
  } // cleanup

}; // class global_object_wrapper_u

} // namespace execution
} // namespace flecsi
