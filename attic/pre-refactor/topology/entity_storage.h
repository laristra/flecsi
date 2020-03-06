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

#include <flecsi/runtime/flecsi_runtime_entity_storage_policy.h>

namespace flecsi {
namespace topology {

// the following type definitions define topological storage types
// as a dependent on the runtime model

template<typename T>
using entity_storage_t = FLECSI_RUNTIME_ENTITY_STORAGE_TYPE<T>;

using offset_storage_t = FLECSI_RUNTIME_OFFSET_STORAGE_TYPE;

} // namespace topology
} // namespace flecsi
