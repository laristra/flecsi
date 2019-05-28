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

#include <cinchlog.h>

#include <type_traits>

#include <flecsi/execution/context.h>
#include <flecsi/utils/mpi_type_traits.h>

clog_register_tag(reduction_wrapper);

namespace flecsi {
namespace execution {

template<size_t HASH, typename TYPE>
struct reduction_wrapper_u {

  /*!
    TODO: Register the user-defined reduction operator with the runtime.
   */

  static void registration_callback() {} // registration_callback

}; // struct reduction_wrapper_u

} // namespace execution
} // namespace flecsi
