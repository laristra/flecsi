/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif
#include "flecsi/data/reference.hh"
#include "flecsi/topo/set/types.hh"

namespace flecsi {
namespace topo {

//----------------------------------------------------------------------------//
// Mesh topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

template<typename POLICY_TYPE>
struct set : set_base_t, data::reference_base {}; // struct set

} // namespace topo
} // namespace flecsi
