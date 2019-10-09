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
#else
#include "flecsi/data/reference.hh"
#include <flecsi/topology/set/types.hh>
#endif

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Mesh topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

template<typename POLICY_TYPE>
struct set_topology : public set_topology_base_t,
                      public data::reference_base {}; // struct set_topology

} // namespace topology
} // namespace flecsi
