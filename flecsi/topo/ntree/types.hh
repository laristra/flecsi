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

#include "flecsi/topo/index.hh"
#include "flecsi/util/geometry/point.hh"

namespace flecsi {
namespace topo {

/*
 *
 */
template<size_t DIM, typename T, class KEY>
class sort_entity
{
  using point_t = util::point<T, DIM>;
  using key_t = KEY;
  using type_t = T;

public:
  sort_entity() {}

  point_t coordinates() const {
    return coordinates_;
  }
  key_t key() const {
    return key_;
  }
  int64_t id() const {
    return id_;
  }
  type_t mass() const {
    return mass_;
  }
  type_t radius() const {
    return radius_;
  }

  void set_coordinates(const point_t & coordinates) {
    coordinates_ = coordinates;
  }
  void set_key(const key_t & key) {
    key_ = key;
  }
  void set_id(const int64_t & id) {
    id_ = id;
  }
  void set_mass(const type_t & mass) {
    mass_ = mass;
  }
  void set_radius(const type_t & radius) {
    radius_ = radius;
  }

  template<size_t D, typename TY, class K>
  friend std::ostream & operator<<(std::ostream & os,
    const sort_entity<D, TY, K> & e);

private:
  point_t coordinates_;
  key_t key_;
  int64_t id_;
  type_t mass_;
  type_t radius_;
}; // class sort_entity

template<size_t DIM, typename T, class KEY>
std::ostream &
operator<<(std::ostream & os, const sort_entity<DIM, T, KEY> & e) {
  os << "Coords: " << e.coordinates() << " Mass: " << e.mass()
     << " Radius: " << e.radius() << " Key: " << e.key() << " Id: " << e.id();
  return os;
}

template<size_t DIM, typename T, class KEY>
class hcell_base_t
{

  const static size_t dimension = DIM;
  using type_t = T;
  using key_t = KEY;

  enum type_displ : int {
    CHILD_DISPL = 0,
    LOCALITY_DISPL = 1 << dimension,
    REQUESTED_DISPL = (1 << dimension) + 2,
    NCHILD_RECV_DISPL = (1 << dimension) + 3
  };
  enum type_mask : int {
    CHILD_MASK = 0b11111111,
    LOCALITY_MASK = 0b11 << LOCALITY_DISPL,
    REQUESTED_MASK = 0b1 << REQUESTED_DISPL,
    NCHILD_RECV_MASK = 0b1111 << NCHILD_RECV_DISPL
  };
  enum type_locality : int { LOCAL = 0, NONLOCAL = 1, SHARED = 2 };

public:
  hcell_base_t() = default;

  hcell_base_t(const key_t & key) : key_(key) {}

  key_t key() const {
    return key_;
  }

  size_t ent_idx() const {
    assert(is_ent_);
    assert(!is_node_);
    return idx_;
  }
  size_t node_idx() const {
    assert(!is_ent_);
    assert(is_node_);
    return idx_;
  }
  void set_key(const key_t & key) {
    key_ = key;
  }
  void set_ent_idx(const int & idx) {
    is_ent_ = true;
    is_node_ = false;
    idx_ = idx;
  }
  void set_node_idx(const int & idx) {
    is_ent_ = false;
    is_node_ = true;
    idx_ = idx;
  }
  void set_node() {
    is_ent_ = false;
    is_node_ = true;
    idx_ = 0;
  }
  void unset() {
    is_ent_ = false;
    is_node_ = false;
    idx_ = 0;
  }
  void add_child(const int & c) {
    type_ |= (1 << c);
  }
  bool is_ent() const {
    return is_ent_;
  }

  bool is_node() const {
    return is_node_;
  }

  bool is_unset() {
    return !is_node_ && !is_ent_;
  }
  unsigned int type() {
    return type_;
  }

  bool has_child(const std::size_t & c) const {
    return type_ & (1 << c);
  }

  template<size_t DD, typename TT, class KK>
  friend std::ostream & operator<<(std::ostream & os,
    const hcell_base_t<DD, TT, KK> & hb);

private:
  key_t key_;
  size_t idx_ = 0;
  bool is_ent_ = false;
  bool is_node_ = false;
  unsigned int type_ = 0;
};

template<size_t D, typename T, class K>
std::ostream &
operator<<(std::ostream & os, const hcell_base_t<D, T, K> & hb) {
  hb.is_node() ? os << "hb: node " : os << "hb: ent ";
  os << hb.key_ << "-" << hb.idx_;
  return os;
}

template<size_t DIM, typename T, class KEY>
class node
{ node() = default; };

} // namespace topo
} // namespace flecsi
