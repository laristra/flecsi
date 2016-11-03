#include <cinchtest.h>
#include <iostream>
#include <cmath>

#include "flecsi/topology/tree_topology.h"

using namespace std;
using namespace flecsi;
using namespace topology;

class tree_policy{
public:
  using tree_t = tree_topology<tree_policy>;

  using branch_int_t = uint64_t;

  static const size_t dimension = 3;

  using element_t = double;

  using point_t = point<element_t, dimension>;

  class entity : public tree_entity<branch_int_t, dimension>{
  public:
    entity(const point_t& p)
    : coordinates_(p){}

    const point_t& coordinates() const{
      return coordinates_;
    }

    void move(const point_t& offset){
      coordinates_ += offset;
    }

    private:
      point_t coordinates_;
  };

  using entity_t = entity;

  class branch : public tree_branch<branch_int_t, dimension>{
  public:
    branch(){}

    void insert(entity_t* ent){
      ents_.push_back(ent);
      
      if(ents_.size() > 1){
        refine();
      }
    }

    void remove(entity_t* ent){
      auto itr = std::find(ents_.begin(), ents_.end(), ent);
      assert(itr != ents_.end());
      ents_.erase(itr);
      
      if(ents_.empty()){
        coarsen();
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

    point_t
    coordinates(const std::array<point<element_t, dimension>, 2>& range) const{
      point_t p;
      id().coordinates(range, p);
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

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

using tree_topology_t = tree_topology<tree_policy>;
using entity_t = tree_topology_t::entity;
using point_t = tree_topology_t::point_t;
using branch_t = tree_topology_t::branch_t;
using branch_id_t = tree_topology_t::branch_id_t;
using element_t = tree_topology_t::element_t;

TEST(tree_topology, neighbors) {
  tree_topology_t t;

  vector<entity_t*> ents;

  size_t n = 5000;

  for(size_t i = 0; i < n; ++i){
    point_t p = {uniform(0, 1), uniform(0, 1), uniform(0, 1)};
    auto e = t.make_entity(p);
    t.insert(e);
    ents.push_back(e);
  }

  for(size_t i = 0; i < n; ++i){
    auto ent = ents[i];

    auto ns = t.find_in_radius(ent->coordinates(), 0.10);

    set<entity_t*> s1;
    s1.insert(ns.begin(), ns.end());

    set<entity_t*> s2;

    for(size_t j = 0; j < n; ++j){
      auto ej = ents[j];

      if(distance(ent->coordinates(), ej->coordinates()) < 0.10){
        s2.insert(ej);
      }
    }

    ASSERT_TRUE(s1 == s2);
  }
}
