/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

/*!
  \file tree_entity.h
  \authors jloiseau@lanl.gov
  \date jul. 2018
 */

#pragma once

/*! @file */

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <flecsi/concurrency/thread_pool.h>
#include <flecsi/data/data_client.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/geometry/point.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/types.h>

#include <flecsi/topology/tree/tree_utils.h>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * class tree_entity_base_u
 *----------------------------------------------------------------------------*/
template<class>
class tree_topology_base_u;

// Aliases for backward compatibility
using tree_entity_base_ = entity_base_;

using tree_entity_base_t = entity_base_u<0>;

/*----------------------------------------------------------------------------*
 * class tree_entity_u
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class tree_entity_u tree_types.h
//! \brief tree_entity_u parameterizes a tree entity base with its dimension and
//! number of domains
//!
//! \tparam DIM The dimension of the entity.
//! \tparam KEY The type of key used for the tree construction
//-----------------------------------------------------------------//

template<size_t DIM, class KEY>
class tree_entity_u : public tree_entity_base_t
{
public:
  enum LOCALITY : int { LOCAL = 0, NON_LOCAL = 1, SHARED = 2 };

  static constexpr size_t dimension = DIM;
  using point_t = point_u<double, dimension>;
  using id_t = flecsi::utils::id_t;
  using key_t = KEY;

  tree_entity_u() : locality_(LOCAL) {}
  ~tree_entity_u() {}

  tree_entity_u(const point_t & coordinates) : coordinates_(coordinates) {}

  // Setters
  void set_coordinates(const point_t & coordinates) {
    coordinates_ = coordinates;
  }
  void set_global_id(id_t id) {
    id_ = id;
  }
  void set_key(const key_t & key) {
    key_ = key;
  }

  // Getters
  key_t key() {
    return key_;
  }
  id_t global_id() {
    return id_;
  }
  point_t coordinates() {
    return coordinates_;
  }
  LOCALITY locality() {
    return locality_;
  }
  size_t owner() {
    return owner_;
  }
  double mass() {
    return mass_;
  }
  double radius() {
    return radius_;
  }

  template<size_t A, class B>
  friend std::ostream & operator<<(std::ostream & os,
    const tree_entity_u<A, B> & dt);

private:
  point_t coordinates_;
  double mass_;
  double radius_;
  key_t key_;
  size_t owner_;
  id_t id_;

  LOCALITY locality_;

}; // class tree_entity_u

template<size_t DIM, class KEY>
std::ostream &
operator<<(std::ostream & os, const tree_entity_u<DIM, KEY> & e) {
  os << "Entity: p: " << e.coordinates_ << " m: " << e.mass_;
  os << " id: " << e.id_.entity() << " key: " << e.key_;
  return os;
}

//-----------------------------------------------------------------//
//! \class tree_entity_holder_u tree_types.h
//! \brief tree_entity_holder_u parameterizes a tree entity base with its
//! dimension and number of domains
//!
//! \tparam DIM The dimension of the entity.
//! \tparam KEY The type of key used for the tree construction
//-----------------------------------------------------------------//

template<size_t DIM, class KEY>
class tree_entity_holder_u : public tree_entity_base_t
{
public:
  static constexpr size_t dimension = DIM;
  using point_t = point_u<double, dimension>;
  using id_t = flecsi::utils::id_t;
  using key_t = KEY;

  tree_entity_holder_u() {}
  ~tree_entity_holder_u() {}

  tree_entity_holder_u(const point_t & p) : coordinates_(p) {}

private:
  point_t coordinates_;
  double mass_;
  double radius_;
  key_t key_;
  int owner_;
}; // class tree_entity_u

/*----------------------------------------------------------------------------*
 * class tree_branch_t
 *----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------//
//! \class tree_branch_u tree_types.h
//!
//! \brief tree_branch_u parametrizes a tree ebranch base with its dimension
//! and number of domain
//!
//! \tparam DIM Dimension
//! \tparam TREE_ENTITY_TYPE The type of entities stored in the branches
//! \tparam KEY The type of key to represent this branch
//----------------------------------------------------------------------------//

template<size_t DIM, class TREE_ENTITY_TYPE, class KEY>
class tree_branch_u : public tree_entity_base_t
{
public:
  enum LOCALITY : int { LOCAL = 0, NON_LOCAL = 1, SHARED = 2 };

  static constexpr size_t dimension = DIM;
  using entity_id_t = typename TREE_ENTITY_TYPE::id_t;
  using id_t = flecsi::utils::id_t;
  using point_t = point_u<double, dimension>;
  using key_t = KEY;

  tree_branch_u() : size_(0), is_leaf_(true) {}
  tree_branch_u(const key_t & key) : size_(0), is_leaf_(true) {
    key_ = key;
  }
  ~tree_branch_u() {}

  // Setter
  void set_leaf(const bool & is_leaf) {
    is_leaf_ = is_leaf;
  }
  void set_bit_child(const char & bit_child) {
    bit_child_ = bit_child;
  }
  void set_coordinates(const point_t & coordinates) {
    coordinates_ = coordinates;
  }
  void set_bmin(const point_t & bmin) {
    bmin_ = bmin;
  }
  void set_bmax(const point_t & bmax) {
    bmax_ = bmax;
  }
  void set_mass(const double & mass) {
    mass_ = mass;
  }
  void set_sub_entities(const size_t & sub_entities) {
    sub_entities_ = sub_entities;
  }

  // Getter
  key_t key() {
    return key_;
  }
  uint32_t size() {
    return size_;
  }
  size_t owner() {
    return owner_;
  }
  size_t sub_entities() {
    return sub_entities_;
  }
  bool requested() {
    return requested_;
  }
  bool ghosts_local() {
    return ghosts_local_;
  }
  LOCALITY locality() {
    return locality_;
  }
  double mass() {
    return mass_;
  }
  point_t coordinates() {
    return coordinates_;
  }
  point_t bmin() {
    return bmin_;
  }
  point_t bmax() {
    return bmax_;
  }
  char bit_child() {
    return bit_child_;
  }

  // Check
  bool is_leaf() {
    return is_leaf_;
  }

  /**
   * Add an entity in this branch entities
   */
  void insert(const entity_id_t & id) {
    entities_[size_++] = id;
  }

  entity_id_t * begin() {
    return entities_;
  }
  entity_id_t * end() {
    return entities_ + size_;
  }

  void clear() {
    size_ = 0;
  }
  void add_bit_child(char bit) {
    bit_child_ = bit_child_ | (1 << bit);
  }
  bool as_child(int bit) {
    return bit_child_ & (1 << bit);
  }

private:
  entity_id_t entities_[1 << dimension];
  point_t coordinates_;
  double mass_;
  point_t bmin_;
  point_t bmax_;
  key_t key_;
  bool is_leaf_;
  uint32_t size_;
  char bit_child_;
  size_t sub_entities_;
  size_t owner_;

  bool requested_;
  bool ghosts_local_;

  LOCALITY locality_;
};

/*----------------------------------------------------------------------------*
 * class tree_topology_base_u
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class tree_topology_base_u tree_topology.h
//! \brief contains methods and data about the tree topology
//-----------------------------------------------------------------//

class tree_topology_base_t
{
};

template<class STORAGE_TYPE>
class tree_topology_base_u : public data::data_client_t,
                             public tree_topology_base_t
{
public:
  using id_t = utils::id_t;

  // Default constructor
  tree_topology_base_u(STORAGE_TYPE * ms = nullptr) : ms_(ms) {}

  // Don't allow the tree to be copied or copy constructed
  tree_topology_base_u(const tree_topology_base_u & m) : ms_(m.ms_) {}
  tree_topology_base_u & operator=(const tree_topology_base_u &) = delete;

  /// Allow move operations
  tree_topology_base_u(tree_topology_base_u &&) = default;

  //! override default move assignement
  tree_topology_base_u & operator=(tree_topology_base_u && o) {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };

  STORAGE_TYPE * set_storage(STORAGE_TYPE * ms) {
    ms_ = ms;
    return ms_;
  } // set_storage

  STORAGE_TYPE * storage() {
    return ms_;
  } // set_storage

  void clear_storage() {
    ms_ = nullptr;
  } // clear_storage

  void delete_storage() {
    delete ms_;
  } // delete_storage

  //-----------------------------------------------------------------//
  //! This method should be called to construct and entity rather than
  //! calling the constructor directly. This way, the ability to have
  //! extra initialization behavior is reserved.
  //-----------------------------------------------------------------/
  template<class T, class... S>
  T * make(S &&... args) {
    std::cout << "Calling make: " << ms_ << std::endl;
    return ms_->template make<T>(std::forward<S>(args)...);
  } // make

protected:
  STORAGE_TYPE * ms_ = nullptr;

}; // tree_topology_base_u

} // namespace topology
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
