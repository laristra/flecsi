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
#include "flecsi/data/copy_plan.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/core.hh" // base
#include "flecsi/topo/ntree/coloring.hh"
#include "flecsi/topo/ntree/types.hh"
#include "flecsi/util/hashtable.hh"

#include <fstream>
#include <iomanip>
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
    size_t nnodes = 0;
  };

  template<std::size_t>
  struct access;

  ntree(const coloring & c)
    : part{{make_repartitioned<Policy, entities>(c.nparts_,
              make_partial<allocate>(c.entities_offset_)),
        make_repartitioned<Policy, nodes>(c.nparts_,
          make_partial<allocate>(c.nodes_offset_)),
        make_repartitioned<Policy, hashmap>(c.nparts_,
          make_partial<allocate>(c.hmap_offset_)),
        make_repartitioned<Policy, tree_data>(c.nparts_,
          make_partial<allocate>(c.tdata_offset_))}},

      cp_data_tree(
        *this,
        {c.nparts_, {{1, 3}}},
        [&] {
          data::copy_plan::Points dst_ptrs(c.nparts_);
          std::size_t colors = c.nparts_;
          for(std::size_t i = 0; i < colors; ++i) {
            auto & v = dst_ptrs[i];
            v.resize(3);
            v[1] = data::points::make(i == 0 ? i : i - 1, 0);
            v[2] = data::points::make(i == colors - 1 ? i : i + 1, 0);
          }
          return dst_ptrs;
        }(),
        util::constant<tree_data>()) {}
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
    std::pair<key_t, hcell_t>>::template definition<Policy, hashmap>
    hcells;

  // Tdata field
  static inline const typename field<ntree_data>::template definition<Policy,
    tree_data>
    data_field;

  // --------------------------------------------------------------------------

  // Use to reference the index spaces by id
  util::key_array<repartitioned, index_spaces> part;

  data::copy_plan cp_data_tree;
  std::optional<data::copy_plan> cp_entities;

  void exch() {
    cp_data_tree.issue_copy(data_field.fid);
  }

  std::size_t colors() const {
    return part.front().colors();
  }

  template<index_space S>
  data::region & get_region() {
    return part.template get<S>();
  }

  template<index_space S>
  const data::partition & get_partition(field_id_t) const {
    return part.template get<S>();
  }

private:
  const static size_t nchildren_ = 1 << dimension;
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
  void send(F && f) {
    e_coordinates.topology_send(f);
    e_radius.topology_send(f);
    e_keys.topology_send(f);
    n_coordinates.topology_send(f);
    n_radius.topology_send(f);
    n_keys.topology_send(f);
    data_field.topology_send(f);
    hcells.topology_send(f);
  }

  using hmap_t = util::hashtable<ntree::key_t, ntree::hcell_t, ntree::hash_f>;

  void exchange_boundaries() {
    data_field(0).max_depth = 0;
    data_field(0).nents = e_coordinates.span().size();
    data_field(0).lobound = process() == 0 ? key_t::min() : e_keys(0);
    data_field(0).hibound = process() == processes() - 1
                              ? key_t::max()
                              : e_keys(data_field(0).nents - 1);
  }

  void make_tree() {
    // Cstr htable
    hmap_t hmap(hcells.span());

    // Create the tree
    size_t size = run::context::instance().colors(); // colors();
    size_t rank = run::context::instance().color(); // color();

    /* Exchange high and low bound */
    key_t lokey = e_keys(0);
    key_t hikey = e_keys(data_field(0).nents - 1);

    const auto hibound =
      rank == size - 1 ? key_t::max() : data_field(2).lobound;
    const auto lobound = rank == 0 ? key_t::min() : data_field(1).hibound;
    assert(lobound <= lokey);
    assert(hibound >= hikey);

    // Add the root
    hmap.insert(key_t::root(), key_t::root());
    auto root_ = hmap.find(key_t::root());
    assert(root_ != hmap.end());

    size_t current_depth = key_t::max_depth();
    // Entity keys, last and current
    key_t lastekey = key_t(0);
    if(rank != 0)
      lastekey = lobound;
    // Node keys, last and Current
    key_t lastnkey = key_t::root();
    key_t nkey, loboundnode, hiboundnode;
    // Current parent and value
    hcell_t * parent = nullptr;
    bool old_is_ent = false;

    bool iam0 = rank == 0;
    bool iamlast = rank == size - 1;

    // The extra turn in the loop is to finish the missing
    // parent of the last entity
    for(size_t i = 0; i <= e_keys.span().size(); ++i) {
      const key_t ekey = i < e_keys.span().size() ? e_keys(i) : hibound;
      nkey = ekey;
      nkey.pop(current_depth);
      bool loopagain = false;
      // Loop while there is a difference in the current keys
      while(nkey != lastnkey || (iamlast && i == e_keys.span().size())) {
        loboundnode = lobound;
        loboundnode.pop(current_depth);
        hiboundnode = hibound;
        hiboundnode.pop(current_depth);
        if(loopagain && (iam0 || lastnkey > loboundnode) &&
           (iamlast || lastnkey < hiboundnode)) {
          // This node is done, we can compute CoFM
          finish_subtree(lastnkey, hmap);
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

      parent = &(hmap.find(lastnkey)->second);
      old_is_ent = parent->is_ent();
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
        parent->add_child(bit);
        parent->unset();
        hmap.insert(nkey, nkey);
        parent = &(hmap.find(nkey)->second);
      } // while

      // Recover deleted entity
      if(old_is_ent) {
        int bit = lastnkey.last_value();
        parent->add_child(bit);
        parent->unset();
        auto it = hmap.insert(lastnkey, lastnkey);
        it->second.set_ent_idx(i - 1);
      } // if

      if(i < e_keys.span().size()) {
        // Insert the new entity
        int bit = nkey.last_value();
        parent->add_child(bit);
        auto it = hmap.insert(nkey, nkey);
        it->second.set_ent_idx(i);
      } // if

      // Prepare next loop
      lastekey = ekey;
      lastnkey = nkey;
      data_field(0).max_depth =
        std::max(data_field(0).max_depth, current_depth);

    } // for
  }

  // Count number of entities to send to each other color
  void boundaries_size() {
    hmap_t hmap(hcells.span());

    int send_nodes = 0;
    int send_ents = 0;
    std::vector<hcell_t *> queue;
    std::vector<hcell_t *> nqueue;
    queue.push_back(&hmap.find(key_t::root())->second);
    while(!queue.empty()) {
      for(hcell_t * cur : queue) {
        key_t nkey = cur->key();
        if(cur->is_unset()) {
          assert(cur->type() != 0);
          for(std::size_t j = 0; j < nchildren_; ++j) {
            if(cur->has_child(j)) {
              key_t ckey = nkey;
              ckey.push(j);
              auto it = hmap.find(ckey);
              nqueue.push_back(&it->second);
            } // if
          } // for
        }
        else {
          if(cur->is_node()) {
            ++send_nodes;
            // std::cout << " NODE: " << cur->key() << std::endl;
            // cofm_t * cofm = get_node(cur);
            // TODO: check if initializing nchildren with 0 is OK here
            // nodes.emplace_back(cur->owner(), cur->key(), *cofm, 0);
          }
          else {
            ++send_ents;
            // std::cout << " ENT: " << cur->key() << std::endl;
            // entity_t * ent = get_entity(cur);send_nodes
            // entities.emplace_back(cur->owner(), cur->key(), *ent);
          } // if
        } // else
      } // for
      queue = std::move(nqueue);
      nqueue.clear();
    } // while
  }

  void finish_subtree(const key_t & key, hmap_t & hmap) {
    auto it = hmap.find(key);
    assert(it != hmap.end());
    hcell_t * n = &(it->second);
    if(n->is_ent())
      return;
    std::size_t cnode = data_field(0).nnodes++;
    n->set_node_idx(cnode);
    n_keys(cnode) = key;
  }

  // Draw the tree
  void graphviz_draw(int num) {
    int rank = process();
    std::ostringstream fname;
    fname << "output_graphviz_" << std::setfill('0') << std::setw(3) << rank
          << "_" << std::setfill('0') << std::setw(3) << num << ".gv";
    std::ofstream output;
    output.open(fname.str().c_str());
    output << "digraph G {\nforcelabels=true;\n";
    std::stack<hcell_t *> stk;
    hmap_t hmap(hcells.span());

    stk.push(&(hmap.find(key_t::root())->second));

    while(!stk.empty()) {
      hcell_t * cur = stk.top();
      stk.pop();
      if(cur->is_node()) {
        // int sub_ent = 0;
        // int idx = cur->node_idx();
        // cofm_t * c = cur->is_shared() ? &shared_nodes_[idx] : &cofm_[idx];
        output << std::oct << cur->key() << std::dec << " [label=\"" << std::oct
               << cur->key() << std::dec << "\"];\n";
        // if(cur->is_shared()) {
        //  output << std::oct << cur->key() << std::dec
        //         << " [shape=circle,color=green]" << std::endl;
        //}
        // else {
        output << std::oct << cur->key() << std::dec
               << " [shape=circle,color=blue]\n";

        // Add the child to the stack and add for display
        for(size_t i = 0; i < nchildren_; ++i) {
          key_t ckey = cur->key();
          ckey.push(i);
          auto it = hmap.find(ckey);
          if(it != hmap.end()) {
            stk.push(&it->second);
            output << std::oct << cur->key() << "->" << it->second.key()
                   << std::dec << std::endl;
          }
        }
      }
      else if(cur->is_ent()) {
        output << std::oct << cur->key() << std::dec << " [label=\"" << std::oct
               << cur->key() << std::dec << "\"];\n";
        // if(cur->is_shared()) {
        //  output << std::oct << cur->key() << std::dec
        //         << " [shape=circle,color=grey]" << std::endl;
        //}
        // else {
        output << std::oct << cur->key() << std::dec
               << " [shape=circle,color=red]\n";
        //}
      }
      else if(cur->is_unset()) {
        output << std::oct << cur->key() << std::dec << " [label=\"" << std::oct
               << cur->key() << std::dec << "\"];\n";
        // if(cur->is_shared()) {
        //  output << std::oct << cur->key() << std::dec
        //         << " [shape=circle,color=grey]" << std::endl;
        //}
        // else {
        output << std::oct << cur->key() << std::dec
               << " [shape=circle,color=black]\n";
        //}
        for(size_t i = 0; i < nchildren_; ++i) {
          key_t ckey = cur->key();
          ckey.push(i);
          auto it = hmap.find(ckey);
          if(it != hmap.end()) {
            stk.push(&it->second);
            output << std::oct << cur->key() << "->" << it->second.key()
                   << std::dec << std::endl;
          }
        }
      } // if
    } // while
    output << "}\n";
    output.close();
  }
};

template<>
struct detail::base<ntree> {
  using type = ntree_base;
};

} // namespace topo
} // namespace flecsi
