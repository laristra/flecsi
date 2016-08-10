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

#ifndef flecsi_basic_tree_h
#define flecsi_basic_tree_h

#include "flecsi/topology/tree_topology.h"

namespace flecsi{

template<class P>
class basic_tree_policy{
public:
  using branch_int_t = uint64_t;

  static const size_t dimension = P::dimension;

  using element_t = typename P::floating_type;

  using point_t = point<element_t, dimension>;

  using range_t = std::pair<element_t, element_t>;

  static constexpr range_t coordinate_range = P::coordinate_range;

  class entity : public topology::tree_entity<branch_int_t, dimension>{
  public:
    entity(const point_t& p)
      : coordinates_(p){}

    const point_t& coordinates() const{
      return coordinates_;
    }

  private:
    point_t coordinates_;
  };

  using entity_t = entity;

  class branch : public topology::tree_branch<branch_int_t, dimension>{
  public:
    using super_ = topology::tree_branch<branch_int_t, dimension>;

    branch(){}

    void insert(entity_t* ent){
      ents_.push_back(ent);
      
      if(ents_.size() > P::max_entities_per_branch){
        super_::refine();
      }
    }

    void remove(entity_t* ent){
      auto itr = std::find(ents_.begin(), ents_.end(), ent);
      assert(itr != ents_.end());
      ents_.erase(itr);
      
      if(ents_.empty()){
        super_::coarsen();
      }
    }

    auto begin(){
      return ents_.begin();
    }

    auto end(){
      return ents_.end();
    }

    void clear(){
      ents_.clear();
    }

    size_t count(){
      return ents_.size();
    }

    point_t coordinates() const{
      point_t p;
      super_::id().coordinates(p);
      return p;
    }

    size_t size(){
      return ents_.size();
    }

  private:
    std::vector<entity_t*> ents_;
  };

  bool should_coarsen(branch* parent){
    return true;
  }

  using branch_t = branch;
};

template<class P>
class basic_tree : public topology::tree_topology<basic_tree_policy<P>>{
public:

};

} // namespace flecsi

#endif // flecsi_basic_tree_h
