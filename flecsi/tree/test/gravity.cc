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

  class body : public tree_entity<branch_int_t, dimension>{
  public:
    body(double mass, const point_t& position, const point_t& velocity)
    : mass_(mass), 
    position_(position),
    velocity_(velocity){}

    const point_t& coordinates() const{
      return position_;
    }

    void interact(const body* b){
      double d = distance(position_, b->position_);
      velocity_ += 1e-9*(b->position_ - position_)/(d*d);
    }

    void update(){
      position_ += velocity_;

      if(position_[0] > 1.0){
        position_[0] = 0;
      }
      else if(position_[0] < 0.0){
        position_[0] = 1.0;
      }
      
      if(position_[1] > 1.0){
        position_[1] = 0;
      }
      else if(position_[1] < 0.0){
        position_[1] = 1.0;
      }
    }

  private:
    point_t position_;
    point_t velocity_;
    double mass_;
  };

  using entity_t = body;

  class branch : public tree_branch<branch_int_t, dimension>{
  public:
    branch(){}

    void insert(body* ent){
      ents_.push_back(ent);
      
      if(ents_.size() > 100){
        refine();
      }
    }

    void remove(body* ent){
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

    point_t coordinates() const{
      point_t p;
      id().coordinates(p);
      return p;
    }

  private:
    std::vector<body*> ents_;
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
using body = tree_topology_t::body;
using point_t = tree_topology_t::point_t;
using branch_t = tree_topology_t::branch_t;
using branch_id_t = tree_topology_t::branch_id_t;

static const size_t N = 10000;
static const size_t TS = 50;

TEST(tree_topology, gravity) {
  tree_topology_t t;

  std::vector<body*> bodies;
  for(size_t i = 0; i < N; ++i){
    double m = uniform(0.0, 0.5);
    point_t p = {uniform(0.0, 1.0), uniform(0.0, 1.0)};
    point_t v = {uniform(0.0, 0.001), uniform(0.0, 0.001)};
    auto bi = t.make_entity(m, p, v);
    bodies.push_back(bi);
    t.insert(bi);
  }

  auto f = [&](body* b, body* b0){
    if(b0 == b){
      return;
    }
    
    b0->interact(b);
  };

  for(size_t ts = 0; ts < TS; ++ts){
    //cout << "---- ts = " << ts << endl;

    for(size_t i = 0; i < N; ++i){
      auto bi = bodies[i];
      t.apply_in_radius(bi->coordinates(), 0.01, f, bi);
    }

    for(size_t i = 0; i < N; ++i){
      auto bi = bodies[i];
      bi->update();
      t.update(bi);
    }
  }
}
