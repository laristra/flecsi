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
#else
#include <stack>
#include <iostream>
#include <fstream>

#include <flecsi/utils/flog/utils.h>

#include <flecsi/data/common/data_reference.h>
#include <flecsi/topology/ntree/storage.h>
#include <flecsi/topology/ntree/types.h>
#include <flecsi/topology/ntree/geometry.h>
#endif

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Mesh topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

 //-----------------------------------------------------------------//
 //! The tree topology is parameterized on a policy P which defines its branch
 //! and entity types.
 //-----------------------------------------------------------------//
template<typename POLICY_TYPE>
struct ntree_topology_u : public ntree_topology_base_t,
                          public data::data_reference_base_t {

public:
  using Policy = POLICY_TYPE;

  // tree storage type definition
  using storage_t = ntree_storage_u<Policy>;
  // tree topology base definition
  using base_t = ntree_topology_base_u<storage_t>;
  // entity ID type
  using id_t = utils::id_t;
  // offset type use by connectivities to give offsets and counts
  using offset_t = utils::offset_t;

  using type_identifier_t = ntree_topology_u;

  // ------- Basic declarations: types and subtypes
  static constexpr size_t dimension = Policy::dimension;
  using element_t = typename Policy::element_t;
  using point_t = typename Policy::point_t;
  using range_t = std::array<point_t, 2>;
  // ------- Space filling curve
  using key_t = typename Policy::filling_curve_t;
  // ------- Tree topology
  using branch_t = typename Policy::tree_branch_;
  using branch_vector_t = std::vector<branch_t *>;
  using tree_entity_t = typename Policy::tree_entity_holder_;
  using entity_t = typename Policy::tree_entity_;
  using entity_vector_t = std::vector<entity_t *>;
  using htable_t = std::unordered_map<key_t, branch_t>;
  using entity_id_t = typename entity_base_u<0>::id_t;
  using geometry_t = ntree_geometry_u<element_t, dimension>;

  static constexpr size_t hash_table_capacity_ = htable_t::hash_capacity_;

  /*!
    Constuct a tree topology with unit coordinates, i.e. each coordinate
    dimension is in range [0, 1].
   */
  ntree_topology_u() {
    max_depth_ = 0;
    // Init the new storage, for now without handler
    base_t::set_storage(new storage_t);
    // Init the branch hash table
    base_t::ms_->init_branches(branch_map_, hash_table_capacity_);
    // Add the root in the branch_map_
    htable_t::insert(base_t::ms_->branch_index_space, key_t::root());
    root_ = htable_t::find(base_t::ms_->branch_index_space, key_t::root());
  }

  ntree_topology_u(const ntree_topology_u & s) : base_t(s) {}

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
    return base_t::ms_->template make_entity(std::forward<S>(args)...);
  }

  /**
   * @brief Make the keys for all the enities present in the tree
   */
  void generate_keys() {
    for(auto & ent : *(base_t::ms_->entity_index_space.storage())) {
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
  branch_t * root() {
    return base_t::ms_->branch_index_space.storage()->begin() + root_id_;
  }

  /**
   * @brief Construct a new tree entity. The tree entity's constructor should
   * not be called directly.
   */
  template<class... S>
  entity_t * make_tree_entity(S &&... args) {
    return base_t::ms_->template make_tree_entity(std::forward<S>(args)...);
  }

  /**
   * @brief Output the current state of the tree
   */
  template<class TREE_POLICY>
  friend std::ostream & operator<<(std::ostream & os,
    const ntree_topology_u<TREE_POLICY> & t);

  /**
   * @brief Build the tree topology in the branch_map_, insert all the local
   * particles of the tree in a serial way
   */
  void build_tree() {
    for(auto & ent : *(base_t::ms_->entity_index_space.storage())) {
      insert(ent);
    }
  }

  /**
   * @brief Insert a particle in the tree. Search for the nearest parent and
   * refine if necessary
   */
  void insert(entity_t & ent) {
    // Find parent closest of the entity
    key_t branch_key = ent.key();
    branch_t * parent = find_parent(branch_key);
    assert(parent != nullptr);

    // It is not a leaf, need to insert intermediate branch
    if(!parent->is_leaf()) {
      // Create the missing branch
      int depth = parent->key().depth() + 1;
      branch_key.truncate(depth);
      int bit = branch_key.last_value();
      parent->add_bit_child(bit);

      // Insert this branch and reinsert
      auto new_parent =
        htable_t::insert(base_t::ms_->branch_index_space, branch_key);
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
    parent = find_parent(branch_key);
  }

  /**
   * @brief get an entity from the storage
   */
  entity_t & get(const entity_id_t & id) {
    return *(base_t::ms_->entity_index_space.storage()->begin() + id.entity());
  }

  /**
   * @brief Find the closest parent of a key
   */
  branch_t * find_parent(key_t key) {
    key.truncate(max_depth_);
    while(key != root()->key()) {
      auto br = htable_t::find(base_t::ms_->branch_index_space, key);
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
  void cofm(branch_t * b = nullptr, bool local = false) {
    if(b == nullptr)
      b = root();
    // Find the sub particles on which we want to work
    std::vector<branch_t *> working_branches;
    std::stack<branch_t *> stk_remaining;
    int level = 5;
    std::stack<branch_t *> stk;
    stk.push(b);
    while(!stk.empty()) {
      branch_t * c = stk.top();
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
      std::stack<branch_t *> stk1;
      std::stack<branch_t *> stk2;
      stk1.push(working_branches[b]);
      while(!stk1.empty()) {
        branch_t * cur = stk1.top();
        stk1.pop();
        stk2.push(cur);
        // Push children to stk1
        if(!cur->is_leaf()) {
          for(int i = 0; i < (1 << dimension); ++i) {
            if(!cur->as_child(i))
              continue;
            branch_t * next = child_(cur, i);
            stk1.push(next);
          }
        }
      }
      // Finish the highest part of the tree in serial
      while(!stk2.empty()) {
        branch_t * cur = stk2.top();
        stk2.pop();
        update_COM(cur, local);
      }
    }
    // Finish the high part of the tree on one thread
    while(!stk_remaining.empty()) {
      branch_t * cur = stk_remaining.top();
      stk_remaining.pop();
      update_COM(cur, local);
    }
  }

  /**
   * @brief Compute the COFM information for a dedicated branch
   */
  void update_COM(branch_t * b, bool local_only = false) {
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
        auto branch = child_(b, i);
        if(branch == nullptr)
          continue;
        nchildren += branch->sub_entities();
        mass += branch->mass();
        if(branch->mass() > 0) {
          for(size_t d = 0; d < dimension; ++d) {
            bmax[d] = std::max(bmax[d], branch->bmax()[d]);
            bmin[d] = std::min(bmin[d], branch->bmin()[d]);
          }
        }
        coordinates += branch->mass() * branch->coordinates();
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
    //clog(trace) << " outputing tree file #" << num << std::endl;

    char fname[64];
    sprintf(fname, "output_graphviz_%02d.gv", num);
    std::ofstream output;
    output.open(fname);
    output << "digraph G {" << std::endl << "forcelabels=true;" << std::endl;

    // Add the legend
    output << "branch [label=\"branch\" xlabel=\"sub_entities,owner,requested,"
              "ghosts_local\"]"
           << std::endl;

    std::stack<branch_t *> stk;
    // Get root
    auto rt = root();
    stk.push(rt);

    while(!stk.empty()) {
      branch_t * cur = stk.top();
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
  branch_t * child_(branch_t * b, const int & i) {
    key_t key = b->key();
    key.push(i);
    return htable_t::find(base_t::ms_->branch_index_space, key);
  }

  /**
   * @brief Refine a branch in the tree during creation of the tree
   */
  void refine_(branch_t * b) {
    key_t pid = b->key();
    size_t depth = pid.depth() + 1;
    // For every children
    char bit_child = 0;
    for(auto ent : *b) {
      key_t k = get(ent).key();
      k.truncate(depth);
      bit_child |= 1 << k.last_value();
      htable_t::insert(base_t::ms_->branch_index_space, k);
    }
    max_depth_ = std::max(max_depth_, depth);
    for(auto ent : *b) {
      insert(get(ent));
    }
    b->set_leaf(false);
    b->clear();
    b->set_bit_child(bit_child);
  }

  size_t max_depth_; //! Current max depth of the tree: deepest branch
  range_t range_; //! Range of the domain to generate the keys
  branch_t branch_map_[hash_table_capacity_]; // Hash table of branches
  static constexpr size_t root_id_ = 1; // Id of the roort in the hashtable
  branch_t * root_ = nullptr;
};

template<class TREE_TYPE>
std::ostream &
operator<<(std::ostream & os, const ntree_topology_u<TREE_TYPE> & t) {
  os << "Tree: range: " << t.range_[0] << "-" << t.range_[1];
  return os;
}

} // namespace topology
} // namespace flecsi
