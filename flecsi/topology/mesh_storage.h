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

#include <flecsi/runtime/flecsi_runtime_topology_policy.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

// Define the runtime model specific mesh storage policy

template<size_t NUM_DIMS, size_t NUM_DOMS, size_t NUM_INDEX_SUBSPACES>
class mesh_storage__ : 
   public FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY<NUM_DIMS, NUM_DOMS,
      NUM_INDEX_SUBSPACES> {};

} // namespace topology
} // namespace flecsi
