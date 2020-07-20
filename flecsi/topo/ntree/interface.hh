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

#include "flecsi/data/topology.hh"
#include "flecsi/data/accessor.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/ntree/coloring.hh"
//#include "flecsi/topo/ntree/geometry.hh"
#include "flecsi/topo/ntree/types.hh"

#include "flecsi/topo/ntree/hash_table.hh"

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
  using hcell_t = hcell_base_t<dimension,type_t,key_t>; 
  using node_t = typename Policy::node_t; 
  using ent_t = typename Policy::ent_t;

  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  struct ntree_data{
    key_t hibound, lobound;
    size_t max_depth = 0;
    size_t nents = 0;  
  }; 

  template<std::size_t>
  struct access;

  template<class F>
  static void fields(F f) {
    f(e_coordinates);
    f(e_radius);
    f(e_keys);
    f(n_coordinates); 
    f(n_radius); 
    f(n_keys); 
    f(data_field); 
    f(hcells); 
  }

  //ntree(const coloring & c)
  //  : part(make_partitions(c,
  //      index_spaces(),
  //      std::make_index_sequence<index_spaces::size>())) {}

//#if 0 
  ntree(const coloring & c)
  : part{
      data::partitioned(
        data::make_region<Policy, entities>(c.global_entities_),
        c.processes_,
        [=](std::size_t i) { return c.entities_offset_[i]; },
        data::disjoint,
        data::complete),
      data::partitioned(
        data::make_region<Policy, nodes>(c.global_nodes_),
        c.processes_,
        [=](std::size_t i) { return c.nodes_offset_[i]; },
        data::disjoint,
        data::complete),
      data::partitioned(
        data::make_region<Policy, hashmap>(c.global_hmap_),
        c.processes_,
        [=](std::size_t i) { return c.hmap_offset_[i]; },
        data::disjoint,
        data::complete),
      data::partitioned(
        data::make_region<Policy, tree_data>(c.processes_),
        c.processes_,
        [=](std::size_t i) { return c.tdata_offset_[i]; },
        data::disjoint,
        data::complete)
      }
  {}
  //#endif 

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
  static inline const typename field<point_t>::template definition<Policy,nodes>
    n_coordinates; 
  static inline const field<double>::definition<Policy, nodes> n_radius;
  static inline const typename field<key_t>::template definition<Policy,
    nodes>
    n_keys;

  // Hmap fields 
  static inline const typename field<hcell_t>::template definition<Policy,hashmap>
    hcells; 

  // Tdata field 
  static inline const typename field<ntree_data>::template definition<Policy, tree_data>
    data_field; 

  // --------------------------------------------------------------------------

  // Use to reference the index spaces by id 
  util::key_array<data::partitioned, index_spaces> part;

  std::size_t colors() const {
    return part.front().colors();
  }

  template<index_space S>
  const data::partition & get_partition() const {
    return part.template get<S>();
  }

private: 

  const size_t nchildren_ = 1<<dimension; 

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

  hash_table<ntree::key_t,ntree::hcell_t> hmap_; 

  void make_tree(){
    hmap_.set_capacity(hcells.size()); 
    // Hashtable implementation and test 
    data_field(0).max_depth = 0;
    data_field(0).nents = e_coordinates.size(); 
    data_field(0).lobound = e_keys(0);
    data_field(0).hibound = e_keys(data_field(0).nents-1);
    std::cout<<data_field(0).lobound<<"-"<<data_field(0).hibound<<std::endl;
    for(int i = 0; i < data_field(0).nents; ++i){
      // Add the entities in the hash_map
      key_t c_key = e_keys(i); 
      hmap_.insert(hcells,c_key,i); 
    }
    std::cout<<"Collisions: "<<hmap_.collision()<<std::endl;
  }
};

template<>
struct detail::base<ntree> {
  using type = ntree_base;
};

} // namespace topo
} // namespace flecsi
