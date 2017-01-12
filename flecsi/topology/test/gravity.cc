#include <cinchtest.h>
#include <iostream>

#include "flecsi/topology/tree_topology.h"
#include "flecsi/concurrency/thread_pool.h"
#include "pseudo_random.h"

using namespace std;
using namespace flecsi;

struct Aggregate{
  Aggregate(){
    center = {0, 0};
    mass = 0;
  }

  double mass;
  point<double, 2> center;
};

class tree_policy{
public:
  using tree_t = topology::tree_topology<tree_policy>;

  using branch_int_t = uint64_t;

  static const size_t dimension = 2;

  using element_t = double;

  using point_t = point<element_t, dimension>;

  class body : public topology::tree_entity<branch_int_t, dimension>{
  public:
    body(double mass, const point_t& position, const point_t& velocity)
    : mass_(mass), 
    position_(position),
    velocity_(velocity){}

    const point_t& coordinates() const{
      return position_;
    }

    double mass() const{
      return mass_;
    }

    void interact(const body* b){
      double d = distance(position_, b->position_);
      velocity_ += 1e-9 * b->mass_ * (b->position_ - position_)/(d*d);
    }

    void interact(Aggregate& a){
      double d = distance(position_, a.center);
      velocity_ += 1e-9 * a.mass * (a.center - position_)/(d*d);
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
    double mass_;
    point_t position_;
    point_t velocity_;
  };

  using entity_t = body;

  class branch : public topology::tree_branch<branch_int_t, dimension>{
  public:
    branch(){}

    void insert(body* ent){
      ents_.push_back(ent);
      
      if(ents_.size() > 100){
        refine();
      }
    }

    void remove(body* ent){
      auto itr = find(ents_.begin(), ents_.end(), ent);
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
      branch_id_t bid = id();
      bid.coordinates(range, p);
      return p;
    }

  private:
    vector<body*> ents_;
  };

  bool should_coarsen(branch* parent){
    return true;
  }

  using branch_t = branch;
};


using tree_topology_t = topology::tree_topology<tree_policy>;
using body = tree_topology_t::body;
using point_t = tree_topology_t::point_t;
using branch_t = tree_topology_t::branch_t;
using branch_id_t = tree_topology_t::branch_id_t;

static const size_t N = 5000;
static const size_t TS = 5;

TEST(tree_topology, gravity) {
  tree_topology_t t;

  thread_pool pool;
  pool.start(8);

  pseudo_random rng;

  vector<body*> bodies;
  for(size_t i = 0; i < N; ++i){
    double m = rng.uniform(0.1, 0.5);
    point_t p = {rng.uniform(0.0, 1.0), rng.uniform(0.0, 1.0)};
    point_t v = {rng.uniform(0.0, 0.001), rng.uniform(0.0, 0.001)};
    auto bi = t.make_entity(m, p, v);
    bodies.push_back(bi);
    t.insert(bi);
  }

#if 0
	// These are unused.
  size_t ix = 0;

  auto f = [&](body* b, body* b0){
    if(b0 == b){
      return;
    }
    
    b0->interact(b);
    ++ix;
  };
#endif

  std::mutex mtx;

  auto g = 
  [&](branch_t* b, size_t depth, vector<Aggregate>& aggs) -> bool{
    if(depth > 4 || b->is_leaf()){    
      auto h = [&](body* bi, Aggregate& agg){
       agg.center += bi->mass() * bi->coordinates();
       agg.mass += bi->mass(); 
      };

      Aggregate agg;
      t.visit_children(pool, b, h, agg);
      mtx.lock();
      aggs.emplace_back(move(agg));
      mtx.unlock();

      return true;
    }

    return false;
  };

  for(size_t ts = 0; ts < TS; ++ts){
    //cout << "---- ts = " << ts << endl;

    vector<Aggregate> aggs;
    t.visit(pool, t.root(), g, aggs);

    for(size_t i = 0; i < N; ++i){
      auto bi = bodies[i];
      auto ents = t.find_in_radius(pool, bi->coordinates(), 0.01);
      for(auto e : ents){
        if(bi != e){
          bi->interact(e);
        }
      }
    }

    for(size_t i = 0; i < N; ++i){
      auto bi = bodies[i];
      
      for(auto& agg : aggs){
        bi->interact(agg);  
      }

      bi->update();
      t.update(bi);
    }
  }
}
