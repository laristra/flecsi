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
#include <flecsi/topology/common/utility_types.hh>
#include <flecsi/utils/geometry/point.hh>
#endif

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// NTree topology.
//----------------------------------------------------------------------------//

/*!
  @ingroup topology
 */

struct ntree_topology_base_t {
  using coloring_t = size_t;

  // add storage 
}; // ntree_topology_base_t

/*----------------------------------------------------------------------------*
 * class tree_entity_base
 *----------------------------------------------------------------------------*/
template<class>
class ntree_topology_base;

// Aliases for backward compatibility
using ntree_entity_base_ = entity_base_;

using ntree_entity_base_t = entity_base<0>;

/*----------------------------------------------------------------------------*
 * class tree_entity
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class tree_entity tree_types.h
//! \brief tree_entity parameterizes a tree entity base with its dimension and
//! number of domains
//!
//! \tparam DIM The dimension of the entity.
//! \tparam KEY The type of key used for the tree construction
//-----------------------------------------------------------------//

template<size_t DIM, class KEY>
class ntree_entity : public ntree_entity_base_t
{
public:
  enum LOCALITY : int { LOCAL = 0, NON_LOCAL = 1, SHARED = 2 };

  static constexpr size_t dimension = DIM;
  using point_t = point<double, dimension>;
  using id_t = flecsi::utils::id_t;
  using key_t = KEY;

  ntree_entity() : locality_(LOCAL) {}
  ~ntree_entity() {}

  ntree_entity(const point_t & coordinates) : coordinates_(coordinates) {}

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
    const ntree_entity<A, B> & dt);

private:
  point_t coordinates_;
  double mass_;
  double radius_;
  key_t key_;
  size_t owner_;
  id_t id_;

  LOCALITY locality_;

}; // class tree_entity

template<size_t DIM, class KEY>
std::ostream &
operator<<(std::ostream & os, const ntree_entity<DIM, KEY> & e) {
  os << "Entity: p: " << e.coordinates_ << " m: " << e.mass_;
  os << " id: " << e.id_.entity() << " key: " << e.key_;
  return os;
}

//-----------------------------------------------------------------//
//! \class tree_entity_holder tree_types.h
//! \brief tree_entity_holder parameterizes a tree entity base with its
//! dimension and number of domains
//!
//! \tparam DIM The dimension of the entity.
//! \tparam KEY The type of key used for the tree construction
//-----------------------------------------------------------------//

template<size_t DIM, class KEY>
class ntree_entity_holder : public ntree_entity_base_t
{
public:
  static constexpr size_t dimension = DIM;
  using point_t = point<double, dimension>;
  using id_t = flecsi::utils::id_t;
  using key_t = KEY;

  ntree_entity_holder() {}
  ~ntree_entity_holder() {}

  ntree_entity_holder(const point_t & p) : coordinates_(p) {}

private:
  point_t coordinates_;
  double mass_;
  double radius_;
  key_t key_;
  int owner_;
}; // class tree_entity

/*----------------------------------------------------------------------------*
 * class tree_node_t
 *----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------//
//! \class tree_node tree_types.h
//!
//! \brief tree_node parametrizes a tree node base with its dimension
//! and number of domain
//!
//! \tparam DIM Dimension
//! \tparam TREE_ENTITY_TYPE The type of entities stored in the nodes
//! \tparam KEY The type of key to represent this node
//----------------------------------------------------------------------------//

template<size_t DIM, class TREE_ENTITY_TYPE, class KEY>
class ntree_node : public ntree_entity_base_t
{
public:
  enum LOCALITY : int { LOCAL = 0, NON_LOCAL = 1, SHARED = 2 };

  static constexpr size_t dimension = DIM;
  using entity_id_t = typename TREE_ENTITY_TYPE::id_t;
  using id_t = flecsi::utils::id_t;
  using point_t = point<double, dimension>;
  using key_t = KEY;

  ntree_node() : size_(0), is_leaf_(true) {}
  ntree_node(const key_t & key) : size_(0), is_leaf_(true) {
    key_ = key;
  }
  ~ntree_node() {}

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
   * Add an entity in this node entities
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
}; // class ntree_node

} // namespace topology
} // namespace flecsi
