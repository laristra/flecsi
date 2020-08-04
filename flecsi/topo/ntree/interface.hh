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
#include "flecsi/topo/ntree/types.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/ntree/coloring.hh"
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

  ntree(const coloring & c)
    : part{
        make_repartitioned<Policy, entities>(
          c.nparts_,
          make_partial<allocate>(c.entities_offset_)),
        make_repartitioned<Policy, nodes>(
          c.nparts_,
          make_partial<allocate>(c.nodes_offset_)),
        make_repartitioned<Policy, hashmap>(
          c.nparts_,
          make_partial<allocate>(c.hmap_offset_)),
        make_repartitioned<Policy, tree_data>(
          c.nparts_,
          make_partial<allocate>(c.tdata_offset_))
        } {}

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
  static inline const typename 
    field<std::pair<key_int_t,hcell_t>>::template definition<
      Policy,
      hashmap>
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
  const data::partition & get_partition() const {
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

  using hmap_t = util::hashtable<
                    ntree::key_int_t,
                    ntree::hcell_t,
                    ntree::hash_f>;

  void exchange_boundaries() {}

  void make_tree() {
    // Cstr htable 
    hmap_t hmap(hcells.span()); 

    // Hashtable implementation and test 
    data_field(0).max_depth = 0;
    data_field(0).nents = e_coordinates.span().size(); 
    data_field(0).lobound = e_keys(0);
    data_field(0).hibound = e_keys(data_field(0).nents-1);
    std::cout<<data_field(0).lobound<<"-"<<data_field(0).hibound<<std::endl;
    
    //----- HASHtTABLE TEST -----------------
    // Add the entities in the hash_map
    for(std::size_t i = 0; i < data_field(0).nents; ++i){
      key_t c_key = e_keys(i); 
      hmap.insert(c_key,c_key,i); 
    }
    std::cout<<"DONE INSERT"<<std::endl<<std::endl;
    // Find all the element in the map (and be sure of the id)
    for(std::size_t i = 0 ; i < data_field(0).nents; ++i){
      key_t c_key = e_keys(i); 
      auto ptr = hmap.find(c_key); 
      assert(ptr != hmap.end()); 
      //assert(ptr->second.ent_idx() == i);
    }
    // Loop over existing elements 
    std::cout<<"hmap values: "<<std::endl;
    int count = 0; 
    //for(auto a: hmap){
    //  std::cout<<count++<<" = "<<a.first<<std::endl;
    //}


    // Clear the hmap_
    hmap.clear();
    //----------------------------------------
#if 0 
    size_t size = run::context::instance().colors(); // colors();
    size_t rank = run::context::instance().color(); // color(); 

    /* Exchange high and low bound */
    key_t lokey = e_keys(0);
    key_t hikey = e_keys(data_field(0).nents - 1);
    
    // \TODO independent task? 
    //exchange_boundaries_(hikey, lokey, hibound_, lobound_);
  
    //assert(data_field(0).lobound <= lokey);
    //assert(data_field(0).hibound >= hikey);

    // Add the root
    hmap_t::insert(hcells,key_t::root(),key_t::root()); 
    auto root_ = hmap_t::find(hcells,key_t::root()); 
    assert(root_ != nullptr); 
    //htable_.emplace(key_t::root(), key_t::root());
    //root_ = htable_.find(key_t::root());

    size_t current_depth = key_t::max_depth();
    // Entity keys, last and current
    key_t lastekey = key_t(0);
    if(rank != 0)
      lastekey = data_field(0).lobound;
    key_t ekey;
    // Node keys, last and Current
    key_t lastnkey = key_t::root();
    key_t nkey, loboundnode, hiboundnode;
    // Current parent and value
    hcell_t * parent = nullptr;
    int oldidx = -1;

    bool iam0 = rank == 0;
    bool iamlast = rank == size - 1;

    // The extra turn in the loop is to finish the missing
    // parent of the last entity
    for(size_t i = 0; i <= e_keys.span().size(); ++i) {
      if(i < e_keys.span().size()) {
        ekey = e_keys(i);
        // Compute the current node key
      }
      else {
        ekey = data_field(0).hibound;
      }
      nkey = ekey;
      nkey.pop(current_depth);
      bool loopagain = false;
      // Loop while there is a difference in the current keys
      while(nkey != lastnkey || (iamlast && i == e_keys.span().size())) {
        loboundnode = data_field(0).lobound;
        loboundnode.pop(current_depth);
        hiboundnode = data_field(0).hibound;
        hiboundnode.pop(current_depth);
        if(loopagain && (iam0 || lastnkey > loboundnode) &&
           (iamlast || lastnkey < hiboundnode)) {
          // This node is done, we can compute CoFM
          assert(false); 
          //finish_(lastnkey, f_cc);
        }
        if(iamlast && lastnkey == key_t::root())
          break;
        loopagain = true;
        current_depth++;
        nkey = ekey;
        nkey.pop(current_depth);
        lastnkey = lastekey;
        lastnkey.pop(current_depth);
      } // while

      if(iamlast && i == e_keys.span().size())
        break;

      parent = hmap_t::find(hcells,lastnkey);
      oldidx = parent->ent_idx();
      // Insert the eventual missing parents in the tree
      // Find the current parent of the two entities
      while(1) {
        current_depth--;
        lastnkey = lastekey;
        lastnkey.pop(current_depth);
        nkey = ekey;
        nkey.pop(current_depth);
        if(nkey != lastnkey)
          break;
        // Add a children
        int bit = nkey.last_value();
        parent->add_child(bit);s
        parent->set_ent_idx(-1);
        hmap_t::insert(hcells, nkey);
        parent = hmap_t::find(hcells,nkey);
      } // while

      // Recover deleted entity
      if(oldidx != -1) {
        int bit = lastnkey.last_value();
        parent->add_child(bit);
        parent->set_ent_idx(-1);
        hmap_t::insert(hcells, lastnkey, i - 1);
      } // if

      if(i < e_keys.span().size()) {
        // Insert the new entity
        int bit = nkey.last_value();
        parent->add_child(bit);
        hmap_t::insert(hcells, nkey, i);
      } // if

      // Prepare next loop
      lastekey = ekey;
      lastnkey = nkey;
      data_field(0).max_depth = std::max(data_field(0).max_depth, current_depth);

    } // for
#endif
  }
};

template<>
struct detail::base<ntree> {
  using type = ntree_base;
};

} // namespace topo
} // namespace flecsi
