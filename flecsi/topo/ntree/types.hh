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

  const size_t dimension = DIM;
  using type_t = T;
  using key_t = KEY;

public:
  hcell_base_t() = default;

  hcell_base_t(const key_t & key, const size_t & ent_idx) {
    key_ = key;
    ent_idx_ = ent_idx;
    node_idx_ = 0;
    is_ent_ = true;
  }

  hcell_base_t & operator=(const hcell_base_t & o) {
    key_ = o.key_;
    node_idx_ = o.node_idx_;
    ent_idx_ = o.ent_idx_;
    is_ent_ = o.is_ent_;
    return *this;
  }

  hcell_base_t(const key_t & key) {
    key_ = key;
    node_idx_ = 0;
    ent_idx_ = 0;
  }

  key_t key() const {
    return key_;
  }

  size_t ent_idx() const {
    assert(is_ent_);
    return ent_idx_;
  }

  void set_key(const key_t & key) {
    key_ = key;
  }

  void set_ent_idx(const int & idx) {
    is_ent_ = true;
    ent_idx_ = idx;
  }

  void add_child(const int &) {
    // todo
  }

  template<size_t DD, typename TT, class KK>
  friend std::ostream & operator<<(std::ostream & os,
    const hcell_base_t<DD, TT, KK> & hb);

private:
  key_t key_;
  size_t node_idx_;
  size_t ent_idx_;
  bool is_ent_;
};

template<size_t D, typename T, class K>
std::ostream &
operator<<(std::ostream & os, const hcell_base_t<D, T, K> & hb) {
  os << "hb: " << hb.key_ << "-" << hb.ent_idx_ << "-" << hb.node_idx_;
  return os;
}

template<size_t DIM, typename T, class KEY>
class node
{ node() = default; };

} // namespace topo
} // namespace flecsi
