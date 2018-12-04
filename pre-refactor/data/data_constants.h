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
namespace data {

//----------------------------------------------------------------------------//
//! Enumeration of supported storage types.
//----------------------------------------------------------------------------//

enum storage_label_type_t : size_t {
  base,
  dense,
  global,
  color,
  sparse,
  ragged,
  scoped,
  tuple,
  local,
  subspace
}; // enum storage_label_type_t

} // namespace data
} // namespace flecsi
