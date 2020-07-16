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

//#if !defined(__FLECSI_PRIVATE__)
//#error Do not include this file directly!
//#else
//#include <flecsi/data/common/data_reference.h>
//#endif

#include <flecsi/topo/structured/box_types.hh>
#include <flecsi/util/typeify.hh> 

//#include "box_types.h"

namespace flecsi {
namespace topology {
namespace structured_impl {


//----------------------------------------------------------------------------//
// Utility types
//----------------------------------------------------------------------------//
template<size_t IS>
using index_space_ = flecsi::util::typeify<size_t, IS>;
/*
template< template<size_t, size_t> class T, size_t N, size_t I>
using T_ = T<N, I>;

template<template<size_t, size_t> class T, size_t N, size_t... Is>
auto gen(std::index_sequence<Is...>) {
   return std::tuple<T_<T, N, Is>...>{};
}

template<template<size_t, size_t> class T, size_t N>
auto gen() {
  return gen<T,N>(std::make_index_sequence<N>{});
}

*/
//----------------------------------------------------------------------------//
// Structured Mesh topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

struct structured_topology_base_t {

  using coloring = flecsi::topology::structured_impl::box_coloring_t; 
}; // structured_mesh_topology_base_t


//----------------------------------------------------------------------------//
// Structured mesh entity types. 
//----------------------------------------------------------------------------//
class structured_mesh_entity_base_t
{
 public:
  using id_t = int64_t; 

  structured_mesh_entity_base_t(){};

  virtual ~structured_mesh_entity_base_t() {}

  size_t id() const
  {
    return local_id_;
  }

  void set_id(const id_t &id)
  {
    local_id_ = id;
  }

  size_t global_id() const
  {
    return global_id_;
  }

  void set_global_id(const id_t &id)
  {
    global_id_ = id;
  }


  template <class POLICY_TYPE>
  friend class structured_topology_u;

 private:
  id_t local_id_; 
  id_t global_id_;
}; // class structured_mesh_entity_base__


template <size_t DIM>
class structured_mesh_entity_u : public structured_mesh_entity_base_t
{
 public:
  static const size_t dimension = DIM;

  structured_mesh_entity_u() : structured_mesh_entity_base_t(){};
  virtual ~structured_mesh_entity_u() {}
}; // class structured_mesh_entity__

} // namespace structured_impl
} // namespace topology
} // namespace flecsi
