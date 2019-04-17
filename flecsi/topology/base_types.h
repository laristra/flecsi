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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

namespace flecsi {
namespace topology {

struct unstructured_mesh_topology_base_t {
  // FIXME Dummy value to compile structure
  using coloring_t = size_t;
}; // struct unstructured_mesh_topology_base_t

struct structured_mesh_topology_base_t {
  // FIXME Dummy value to compile structure
  using coloring_t = size_t;
};

struct ntree_topology_base_t {
  // FIXME Dummy value to compile structure
  using coloring_t = size_t;
};

struct set_topology_base_t {
  // FIXME Dummy value to compile structure
  using coloring_t = size_t;
};

} // namespace topology
} // namespace flecsi
