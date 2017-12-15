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

template<size_t ND, size_t NM>
class mesh_storage__ : public FLECSI_RUNTIME_TOPOLOGY_STORAGE_POLICY<ND, NM> {};

} // namespace topology
} // namespace flecsi
