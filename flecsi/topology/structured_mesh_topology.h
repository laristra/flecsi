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
namespace topology {

///
// \class structured_mesh_topology_u structured_mesh_topology.h
// \brief structured_mesh_topology_u provides...
///
template<typename MT>
class structured_mesh_topology_u
{
public:
  /// Default constructor
  structured_mesh_topology_u() {}

  /// Copy constructor (disabled)
  structured_mesh_topology_u(const structured_mesh_topology_u &) = delete;

  /// Assignment operator (disabled)
  structured_mesh_topology_u & operator=(
    const structured_mesh_topology_u &) = delete;

  /// Destructor
  ~structured_mesh_topology_u() {}

  size_t num_entities(size_t dim, size_t domain = 0) {
    return MT::num_entities(dim, domain);
  } // num_entities

private:
}; // class structured_mesh_topology_u

} // namespace topology
} // namespace flecsi
