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

#include <flecsi/runtime/flecsi_runtime_ntree_topology_policy.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

// Define the runtime model specific tree storage policy

template<class TREE_TYPE>
class tree_storage_u
  : public FLECSI_RUNTIME_TREE_TOPOLOGY_STORAGE_POLICY<TREE_TYPE>
{};

} // namespace topology
} // namespace flecsi
