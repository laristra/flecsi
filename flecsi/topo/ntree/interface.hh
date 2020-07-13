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
#include "flecsi/data/privilege.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/ntree/coloring.hh"
#include "flecsi/topo/ntree/geometry.hh"
#include "flecsi/topo/ntree/storage.hh"
#include "flecsi/topo/ntree/types.hh"

#include <fstream>
#include <iostream>
#include <stack>
#include <type_traits>
#include <unordered_map>

namespace flecsi {
namespace topo {

//----------------------------------------------------------------------------//
// NTree topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

//-----------------------------------------------------------------//
//! The tree topology is parameterized on a policy P which defines its nodes
//! and entity types.
//-----------------------------------------------------------------//
template<typename Policy>
struct ntree : ntree_base {

  // Get types from Policy 
  constexpr static unsigned int dimension = Policy::dimension; 
  using type_t = double; 
  using point_t = util::point<type_t, dimension>;
  using key_t = typename Policy::key_t;

  template<std::size_t>
  struct access;

  template<class F>
  static void fields(F f) {
    f(coordinates);
    f(radius); 
    f(keys); 
  }

  ntree(const coloring & c)
    : ents(data::make_region<Policy, entities>(c.global_entities_),
        c.nparts_,
        [=](std::size_t i) {
          return c.offset_[i];
        },
        data::disjoint,
        data::complete) {}

  static inline const typename field<point_t>::template definition<Policy, entities> coordinates;
  static inline const field<double>::definition<Policy, entities> radius;
  static inline const typename field<key_t>::template definition<Policy, entities> keys;

  template<index_space S>
  auto & get_partition() const {
    return ents;
  }

  std::size_t colors() const {
    return ents.colors();
  }

  data::partitioned ents;

};

template<class Policy>
template<std::size_t Priv>
struct ntree<Policy>::access {
  template<const auto & F>
  using accessor = data::accessor_member<F, Priv>;
  accessor<ntree::coordinates> coordinates;
  accessor<ntree::radius> radius;
  accessor<ntree::keys> keys;

  template<class F>
  void bind(F f) {
    //!!! Same order important (fields())
    f(coordinates);
    f(radius);
    f(keys);
  }

  point_t& get_coordinates(const size_t& i){
    return coordinates(i); 
  }

  double& get_radius(const size_t& i){
    return radius(i); 
  }

  key_t& get_key(const size_t& i){
    return keys(i); 
  }
  
};

template<>
struct detail::base<ntree> {
  using type = ntree_base;
};

} // namespace topo
} // namespace flecsi
