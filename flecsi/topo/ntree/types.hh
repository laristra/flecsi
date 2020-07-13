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

#include "../utility_types.hh"
#include "flecsi/topo/ntree/coloring.hh"
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
  sort_entity(){}

  point_t coordinates() const { return coordinates_; }
  key_t key() const { return key_; }
  int64_t id() const { return id_; }
  type_t mass() const { return mass_; }
  type_t radius() const { return radius_; }

  void set_coordinates(const point_t& coordinates){
    coordinates_ = coordinates; 
  }
  void set_key(const key_t& key){ key_ = key; } 
  void set_id(const int64_t& id){ id_ = id; }
  void set_mass(const type_t& mass) {mass_ = mass;}
  void set_radius(const type_t& radius) { radius_ = radius; }

  template<size_t D, typename TY, class K>
  friend std::ostream& operator<<(std::ostream& os,const sort_entity<D,TY,K>& e); 
  
private: 
  point_t coordinates_;  
  key_t key_; 
  int64_t id_; 
  type_t mass_; 
  type_t radius_; 
}; // class sort_entity

  template<size_t DIM, typename T, class KEY>
  std::ostream&
  operator<<(std::ostream& os, const sort_entity<DIM,T,KEY>& e){
    os<<"Coords: "<<e.coordinates()<<" Mass: "<<e.mass()<<" Radius: "<<e.radius()<<" Key: "<<e.key()<<" Id: "<<e.id(); 
    return os; 
  }


} // namespace topo
} // namespace flecsi
