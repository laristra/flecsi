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

#include "flecsi/data/accessor.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/ntree/coloring.hh"
#include "flecsi/topo/ntree/types.hh"
#include "flecsi/util/hashtable.hh"

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
  using key_int_t = typename Policy::key_int_t;
  using key_t = typename Policy::key_t;
  using node_t = typename Policy::node_t;
  using ent_t = typename Policy::ent_t;
  using hash_f = typename Policy::hash_f;

  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  using type_t = double;
  using point_t = util::point<type_t, dimension>;
  using hcell_t = hcell_base_t<dimension, type_t, key_t>;

  struct ntree_data {
    key_t hibound, lobound;
    size_t max_depth = 0;
    size_t nents = 0;
  };

  template<std::size_t>
  struct access;

  template<class F>
  static void fields(F f, typename Policy::slot & s) {
    f(e_coordinates, s);
    f(e_radius, s);
    f(e_keys, s);
    f(n_coordinates, s);
    f(n_radius, s);
    f(n_keys, s);
    f(data_field, s);
    f(hcells, s);
  }

  ntree(const coloring & c)
    : part{{make_repartitioned<Policy, entities>(c.nparts_,
              make_partial<allocate>(c.entities_offset_)),
        make_repartitioned<Policy, nodes>(c.nparts_,
          make_partial<allocate>(c.nodes_offset_)),
        make_repartitioned<Policy, hashmap>(c.nparts_,
          make_partial<allocate>(c.hmap_offset_)),
        make_repartitioned<Policy, tree_data>(c.nparts_,
          make_partial<allocate>(c.tdata_offset_))}} {}

  // Ntree mandatory fields ---------------------------------------------------

  // Entities fields
  static inline const typename field<point_t>::template definition<Policy,
    entities>
    e_coordinates;
  static inline const field<double>::definition<Policy, entities> e_radius;
  static inline const typename field<key_t>::template definition<Policy,
    entities>
    e_keys;

  // Nodes fields
  static inline const typename field<point_t>::template definition<Policy,
    nodes>
    n_coordinates;
  static inline const field<double>::definition<Policy, nodes> n_radius;
  static inline const typename field<key_t>::template definition<Policy, nodes>
    n_keys;

  // Hmap fields
  static inline const typename field<
    std::pair<key_int_t, hcell_t>>::template definition<Policy, hashmap>
    hcells;

  // Tdata field
  static inline const typename field<ntree_data>::template definition<Policy,
    tree_data>
    data_field;

  // --------------------------------------------------------------------------

  // Use to reference the index spaces by id
  util::key_array<repartitioned, index_spaces> part;

  std::size_t colors() const {
    return part.front().colors();
  }

  template<index_space S>
  const data::partition & get_partition(field_id_t) const {
    return part.template get<S>();
  }

private:
  const size_t nchildren_ = 1 << dimension;
};

template<class Policy>
template<std::size_t Priv>
struct ntree<Policy>::access {
  template<const auto & F>
  using accessor = data::accessor_member<F, Priv>;
  accessor<ntree::e_coordinates> e_coordinates;
  accessor<ntree::e_radius> e_radius;
  accessor<ntree::e_keys> e_keys;
  accessor<ntree::n_coordinates> n_coordinates;
  accessor<ntree::n_radius> n_radius;
  accessor<ntree::n_keys> n_keys;
  accessor<ntree::data_field> data_field;
  accessor<ntree::hcells> hcells;

  template<class F>
  void bind(F f) {
    //!!! Same order as function fields()
    f(e_coordinates);
    f(e_radius);
    f(e_keys);
    f(n_coordinates);
    f(n_radius);
    f(n_keys);
    f(data_field);
    f(hcells);
  }

  using hmap_t =
    util::hashtable<ntree::key_int_t, ntree::hcell_t, ntree::hash_f>;

  void exchange_boundaries() {
    data_field(0).max_depth = 0;
    data_field(0).nents = e_coordinates.span().size();
    data_field(0).lobound = e_keys(0);
    data_field(0).hibound = e_keys(data_field(0).nents - 1);
    std::cout << data_field(0).lobound << "-" << data_field(0).hibound
              << std::endl;
  }

  void make_tree() {
    // Cstr htable
    hmap_t hmap(hcells.span());
    // Build tree
  }
};

template<>
struct detail::base<ntree> {
  using type = ntree_base;
};

} // namespace topo
} // namespace flecsi
