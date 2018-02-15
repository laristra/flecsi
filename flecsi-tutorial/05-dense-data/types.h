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

#include <flecsi/data/dense_accessor.h>

namespace types {

using namespace flecsi;

// This is the definition of the struct_type_t type.

struct struct_type_t {
  double a;
  size_t b;
  double v[3];
}; // struct_type_t

// Here, we define an accessor type to use in the task signatures in our
// example. Notice that the base type "dense_accessor" takes four
// parameters. The first parameter is the type that has been registered on
// the associated index space. The other three parameters specify the
// privileges with which the corresponding data will be accessed.

template<
  size_t SHARED_PRIVILEGES>
using struct_field = dense_accessor<struct_type_t, rw, SHARED_PRIVILEGES, ro>;

} // namespace types
