#include <cinchtest.h>
#include <iostream>

#include "flecsi/tree/tree_topology.h"

using namespace std;
using namespace flecsi;
using namespace tree_topology_dev;

class tree_policy{
public:
  using tree_t = tree_topology<tree_policy>;

  using branch_int_t = uint64_t;

  static const size_t dimension = 2;

  using element_t = double;

  using point_t = point<element_t, dimension>;

  class entity : public tree_entity<branch_int_t, dimension>{
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

  private:
    std::vector<entity_t*> ents_;
  };

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

TEST(tree_topology, insert_find_remove) {
  tree_topology_t t({0.0, 100.0, 0.0, 100.0});

  std::vector<entity_t*> ents;

  for(size_t i = 0; i < 100000; ++i){
    point_t p = {uniform(0.0, 100.0), uniform(0.0, 100.0)};
    auto e = t.make_entity(p);
    t.insert(e);
    ents.push_back(e);
  }

  point_t p = {10.0, 10.0};

  auto s = t.find(p, 1.0);
  for(auto ent : s){
    CINCH_CAPTURE() << ent->coordinates() << endl;
  }

  for(auto e : ents){
    t.remove(e);
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("tree.blessed"));
}
