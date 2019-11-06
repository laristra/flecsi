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
#include "flecsi/data/reference.hh"
#include <flecsi/topology/ntree/coloring.hh>
#include <flecsi/topology/ntree/geometry.hh>
#include <flecsi/topology/ntree/storage.hh>
#include <flecsi/topology/ntree/types.hh>

#include <fstream>
#include <iostream>
#include <stack>
#include <unordered_map>

namespace flecsi {
namespace topology {

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
template<typename POLICY_TYPE>
struct ntree_topology : public ntree_topology_base {

public:
  using Policy = POLICY_TYPE;

  // tree storage type definition
  using storage_t = ntree_storage<Policy>;
  // entity ID type
  using id_t = utils::id_t;
  // offset type use by connectivities to give offsets and counts
  using offset_t = utils::offset_t;

  // ------- Basic declarations: types and subtypes
  static constexpr size_t dimension = Policy::dimension;
  using element_t = typename Policy::element_t;
  using point_t = point<element_t, dimension>;
  using range_t = std::array<point_t, 2>;
  // ------- Space filling curve
  using key_t = typename Policy::key_t;
  // ------- Tree topology
  using node_t = typename Policy::node_t;
  using tree_entity_t = typename Policy::tree_entity_t;
  using entity_t = typename Policy::entity_t;
  using entity_id_t = typename entity_base<0>::id_t;
  using geometry_t = ntree_geometry<element_t, dimension>;

  storage_t nts_;

public:
  /*!
    Constuct a tree topology with unit coordinates, i.e. each coordinate
    dimension is in range [0, 1].
   */
  ntree_topology() {
    max_depth_ = 0;
    // Init the new storage, for now without handler
    // Add the root in the node_map_
    node_map_.emplace(key_t::root(), key_t::root());
    root_ = node_map_.find(key_t::root());
    assert(root_ != node_map_.end());
  }

  ntree_topology(const ntree_topology & s) {}

  /**
   * @brief Set the range of the current domain.
   * This range is the same among all the processes and is used to compute the
   * keys for each particles
   */
  void set_range(const range_t & range) {
    range_ = range;
  }

  /**
   * @brief Construct a new entity. The entity's constructor should not be
   * called directly.
   */
  template<class... S>
  entity_t * make_entity(S &&... args) {
    return nts_.template make_entity(std::forward<S>(args)...);
  }

  /**
   * @brief Make the keys for all the enities present in the tree
   */
  void generate_keys() {
    for(auto & ent : *(nts_.entity_index_space.storage())) {
      ent.set_key(key_t(range_, ent.coordinates()));
    }
  }

  /**
   * @brief Sort the entities present in the tree
   */
  void sort_entities() {
    // See how
  }

  /**
   * Return the root from the hash table
   */
  node_t * root() {
    return root_; // base_t::nts_->node_index_space.storage()->begin() +
                  // root_id_;
  }

  /**
   * @brief Construct a new tree entity. The tree entity's constructor should
   * not be called directly.
   */
  template<class... S>
  entity_t * make_tree_entity(S &&... args) {
    return nts_.template make_tree_entity(std::forward<S>(args)...);
  }

  /**
   * @brief Output the current state of the tree
   */
  template<class TREE_POLICY>
  friend std::ostream & operator<<(std::ostream & os,
    const ntree_topology<TREE_POLICY> & t);

  /**
   * @brief Build the tree topology in the node_map_, insert all the local
   * particles of the tree in a serial way
   */
  void build_tree() {
    for(auto & ent : *(nts_.entity_index_space.storage())) {
      insert(ent);
    }
  }

  /**
   * @brief Insert a particle in the tree. Search for the nearest parent and
   * refine if necessary
   */
  void insert(entity_t & ent) {
    // Find parent closest of the entity
    key_t node_key = ent.key();
    node_t * parent = find_parent(node_key);
    assert(parent != nullptr);

    // It is not a leaf, need to insert intermediate node
    if(!parent->is_leaf()) {
      // Create the missing node
      int depth = parent->key().depth() + 1;
      node_key.truncate(depth);
      int bit = node_key.last_value();
      parent->add_bit_child(bit);

      // Insert this node and reinsert
      node_map_.emplace_back(node_key, node_key);
      auto * new_parent = &(node_map_.find(node_key)->second);
      new_parent->set_leaf(true);
      new_parent->insert(ent.global_id());
    }
    else {
      // Conflict with a children
      if(parent->size() == (1 << dimension)) {
        refine_(parent);
        insert(ent);
      }
      else {
        parent->insert(ent.global_id());
      }
    }
    parent = find_parent(node_key);
  }

  /**
   * @brief get an entity from the storage
   */
  entity_t & get(const entity_id_t & id) {
    return *(nts_.entity_index_space.storage()->begin() + id.entity());
  }

  /**
   * @brief Find the closest parent of a key
   */
  node_t * find_parent(key_t key) {
    key.truncate(max_depth_);
    while(key != root()->key()) {
      auto br = &(node_map_.find(key)->second);
      if(br != nullptr) {
        return br;
      }
      key.pop();
    }
    return root();
  }

  /**
   * @brief Compute the cofm data for the tree using double stack
   */
  void cofm(node_t * b = nullptr, bool local = false) {
    if(b == nullptr)
      b = root();
    // Find the sub particles on which we want to work
    std::vector<node_t *> working_branches;
    std::stack<node_t *> stk_remaining;
    int level = 5;
    std::stack<node_t *> stk;
    stk.push(b);
    while(!stk.empty()) {
      node_t * c = stk.top();
      int cur_level = c->key().depth();
      stk.pop();
      if(c->is_leaf()) {
        working_branches.push_back(c);
      }
      else {
        if(cur_level == level) {
          working_branches.push_back(c);
        }
        else {
          stk_remaining.push(c);
          for(int i = (1 << dimension) - 1; i >= 0; --i) {
            if(!c->as_child(i))
              continue;
            auto next = child_(c, i);
            assert(next != nullptr);
            stk.push(next);
          }
        }
      }
    }

    // Work in parallel on the sub branches
    const int nwork = working_branches.size();

#pragma omp parallel for
    for(int b = 0; b < nwork; ++b) {
      // Find the leave in order in these sub branches
      std::stack<node_t *> stk1;
      std::stack<node_t *> stk2;
      stk1.push(working_branches[b]);
      while(!stk1.empty()) {
        node_t * cur = stk1.top();
        stk1.pop();
        stk2.push(cur);
        // Push children to stk1
        if(!cur->is_leaf()) {
          for(int i = 0; i < (1 << dimension); ++i) {
            if(!cur->as_child(i))
              continue;
            node_t * next = child_(cur, i);
            stk1.push(next);
          }
        }
      }
      // Finish the highest part of the tree in serial
      while(!stk2.empty()) {
        node_t * cur = stk2.top();
        stk2.pop();
        update_COM(cur, local);
      }
    }
    // Finish the high part of the tree on one thread
    while(!stk_remaining.empty()) {
      node_t * cur = stk_remaining.top();
      stk_remaining.pop();
      update_COM(cur, local);
    }
  }

  /**
   * @brief Compute the COFM information for a dedicated branch
   */
  void update_COM(node_t * b, bool local_only = false) {
    // Starting branch
    element_t mass = 0;
    point_t bmax{}, bmin{};
    point_t coordinates{};
    uint64_t nchildren = 0;
    int owner = b->owner();
    for(size_t d = 0; d < dimension; ++d) {
      bmax[d] = -DBL_MAX;
      bmin[d] = DBL_MAX;
    }
    bool full_nonlocal = true, full_local = true;
    if(b->is_leaf()) {
      // For local branches, compute the radius
      int start = -1;
      int end = -1;
      for(auto child : *b) {
        auto ent = get(child);
        owner = ent.owner();
        ++nchildren;
        element_t childmass = ent.mass();
        for(size_t d = 0; d < dimension; ++d) {
          bmax[d] = std::max(bmax[d], ent.coordinates()[d] + ent.radius() / 2.);
          bmin[d] = std::min(bmin[d], ent.coordinates()[d] - ent.radius() / 2.);
        }
        coordinates += childmass * ent.coordinates();
        mass += childmass;
      }
      if(mass > element_t(0))
        coordinates /= mass;
    }
    else {
      for(int i = 0; i < (1 << dimension); ++i) {
        auto node = child_(b, i);
        if(node == nullptr)
          continue;
        nchildren += node->sub_entities();
        mass += node->mass();
        if(node->mass() > 0) {
          for(size_t d = 0; d < dimension; ++d) {
            bmax[d] = std::max(bmax[d], node->bmax()[d]);
            bmin[d] = std::min(bmin[d], node->bmin()[d]);
          }
        }
        coordinates += node->mass() * node->coordinates();
      }
      if(mass > element_t(0))
        coordinates /= mass;
    }
    b->set_sub_entities(nchildren);
    b->set_coordinates(coordinates);
    b->set_mass(mass);
    b->set_bmin(bmin);
    b->set_bmax(bmax);
    assert(nchildren != 0);
  }

  /**
   * @brief Output the tree topology in a file
   * The format is graphviz and can be converted to PDF with dot
   * dot -Tpng file.gv > file.png
   */
  void graphviz(int num) {
    flog(trace) << " outputing tree file #" << num << std::endl;

    char fname[64];
    sprintf(fname, "output_graphviz_%02d.gv", num);
    std::ofstream output;
    output.open(fname);
    output << "digraph G {" << std::endl << "forcelabels=true;" << std::endl;

    // Add the legend
    output << "node [label=\"node\" xlabel=\"sub_entities,owner,requested,"
              "ghosts_local\"]"
           << std::endl;

    std::stack<node_t *> stk;
    // Get root
    auto rt = root();
    stk.push(rt);

    while(!stk.empty()) {
      node_t * cur = stk.top();
      stk.pop();
      if(!cur->is_leaf()) {
        output << cur->key() << " [label=\"" << cur->key() << "\", xlabel=\""
               << cur->sub_entities() << " - " << cur->owner() << " - "
               << cur->requested() << " - " << cur->ghosts_local() << "\"];"
               << std::endl;
        switch(cur->locality()) {
          case 1:
            output << cur->key() << " [shape=circle,color=blue]" << std::endl;
            break;
          case 2:
            output << cur->key() << " [shape=circle,color=red]" << std::endl;
            break;
          case 3:
            output << cur->key() << " [shape=circle,color=green]" << std::endl;
            break;
          default:
            output << cur->key() << " [shape=circle,color=black]" << std::endl;
            break;
        }

        // Add the child to the stack and add for display
        for(size_t i = 0; i < (1 << dimension); ++i) {
          auto br = child_(cur, i);
          if(br == nullptr)
            continue;
          stk.push(br);
          output << std::oct << cur->key() << "->" << br->key() << std::dec
                 << std::endl;
        }
      }
      else {
        output << cur->key() << " [label=\"" << cur->key() << "\", xlabel=\""
               << cur->sub_entities() << " - " << cur->owner() << " - "
               << cur->requested() << " - " << cur->ghosts_local() << "\"];"
               << std::endl;
        switch(cur->locality()) {
          case 1:
            output << cur->key() << " [shape=circle,color=blue]" << std::endl;
            break;
          case 2:
            output << cur->key() << " [shape=circle,color=red]" << std::endl;
            break;
          case 3:
            output << cur->key() << " [shape=circle,color=green]" << std::endl;
            break;
          default:
            output << cur->key() << " [shape=circle,color=black]" << std::endl;
            break;
        }
        for(auto ent : *cur) {
          auto e = get(ent);
          key_t key = e.key();
          key.truncate(max_depth_ + 2);
          output << key << " [label=\"" << key << "\", xlabel=\"" << e.owner()
                 << " - " << e.global_id().entity() << "\"];" << std::endl;

          output << cur->key() << "->" << key << std::endl;
          switch(e.locality()) {
            case 0:
              output << key << " [shape=box,color=black]" << std::endl;
              break;
            case 1:
              output << key << " [shape=box,color=red]" << std::endl;
              break;
            case 2:
              output << key << " [shape=box,color=green]" << std::endl;
              break;
            default:
              output << key << " [shape=circle,color=black]" << std::endl;
              break;
          }
          output << std::dec;
        }
      }
    }
    output << "}" << std::endl;
    output.close();
  }

private:
  /**
   * @brief Return the child i of a key
   */
  node_t * child_(node_t * b, const int & i) {
    key_t key = b->key();
    key.push(i);
    return &(node_map_.find(key)->second);
  }

  /**
   * @brief Refine a node in the tree during creation of the tree
   */
  void refine_(node_t * b) {
    key_t pid = b->key();
    size_t depth = pid.depth() + 1;
    // For every children
    char bit_child = 0;
    for(auto ent : *b) {
      key_t k = get(ent).key();
      k.truncate(depth);
      bit_child |= 1 << k.last_value();
      node_map_.emplace_back(k, k);
    }
    max_depth_ = std::max(max_depth_, depth);
    for(auto ent : *b) {
      insert(get(ent));
    }
    b->set_leaf(false);
    b->clear();
    b->set_bit_child(bit_child);
  }

  size_t max_depth_; //! Current max depth of the tree: deepest node
  range_t range_; //! Range of the domain to generate the keys

  // Hasher for the node id used in the unordered_map data structure
  template<class K>
  struct node_key_hasher__ {
    size_t operator()(const K & k) const noexcept {
      return k.value() & ((1 << 22) - 1);
    }
  };
  std::unordered_map<key_t, node_t, node_key_hasher__<key_t>> node_map_;
  typename std::unordered_map<key_t, node_t, node_key_hasher__<key_t>>::iterator
    root_;

}; // ntree_topology

template<class TREE_TYPE>
std::ostream &
operator<<(std::ostream & os, const ntree_topology<TREE_TYPE> & t) {
  os << "Tree: range: " << t.range_[0] << "-" << t.range_[1];
  return os;
}

} // namespace topology
} // namespace flecsi
