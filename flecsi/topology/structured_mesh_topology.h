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
// \class structured_mesh_topology__ structured_mesh_topology.h
// \brief structured_mesh_topology__ provides...
///
template<typename MT>
class structured_mesh_topology__ {
public:
  /// Default constructor
  structured_mesh_topology__() {}

  /// Copy constructor (disabled)
  structured_mesh_topology__(const structured_mesh_topology__ &) = delete;

  /// Assignment operator (disabled)
  structured_mesh_topology__ &
  operator=(const structured_mesh_topology__ &) = delete;

  /// Destructor
  ~structured_mesh_topology__() {}

  size_t num_entities(size_t dim, size_t domain = 0) {
    return MT::num_entities(dim, domain);
  } // num_entities

private:
}; // class structured_mesh_topology__

} // namespace topology
} // namespace flecsi
