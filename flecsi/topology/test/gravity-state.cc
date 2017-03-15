#include <cinchtest.h>
#include <iostream>

#include "flecsi/topology/tree_topology.h"
#include "flecsi/data/old/old_data.h"
#include "pseudo_random.h"

using namespace std;
using namespace flecsi;
using namespace flecsi::data; // FIXME: What namespaces do we need?

struct state_user_meta_data_t {
  void initialize(){}
};

using state_t = 
  data_t<state_user_meta_data_t, default_data_storage_policy_t>;

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

  using vector_t = point<element_t, dimension>;

  class body : public topology::tree_entity<branch_int_t, dimension>{
  public:
    body(){}

    void init(double mass, const point_t& position, const vector_t& velocity){
      state_t& state = state_t::instance();

      auto am = state.dense_accessor<double, flecsi_internal>("mass");
      auto av = state.dense_accessor<vector_t, flecsi_internal>("velocity");
      auto ap = state.dense_accessor<vector_t, flecsi_internal>("position");
      
      am[id()] = mass;
      ap[id()] = position;
      av[id()] = velocity;
    }

    double mass(){
      state_t& state = state_t::instance();
      auto am = state.dense_accessor<double, flecsi_internal>("mass");
      return am[id()];
    }

    point_t coordinates() const{
      state_t& state = state_t::instance();
      auto ap = state.dense_accessor<point_t, flecsi_internal>("position");
      return ap[id()];
    }

    void interact(const body* b){
      state_t& state = state_t::instance();
      auto av = state.dense_accessor<vector_t, flecsi_internal>("velocity");
      auto am = state.dense_accessor<double, flecsi_internal>("mass");
      auto ap = state.dense_accessor<point_t, flecsi_internal>("position");

      point_t p = ap[id()];
      point_t pb = ap[b->id()];

      double d = distance(p, pb);
      av[id()] += 1e-9 * am[id()] * (pb - p)/(d*d);
    }

    void interact(Aggregate& a){
      state_t& state = state_t::instance();
      auto av = state.dense_accessor<vector_t, flecsi_internal>("velocity");
      auto am = state.dense_accessor<double, flecsi_internal>("mass");
      auto ap = state.dense_accessor<point_t, flecsi_internal>("position");

      point_t p = ap[id()];

      double d = distance(p, a.center);
      
      vector_t& v = av[id()];
      v += 1e-9 * a.mass * (a.center - p)/(d*d);
      av[id()] = v;
    }

    void update(){
      state_t& state = state_t::instance();
      auto av = state.dense_accessor<vector_t, flecsi_internal>("velocity");
      auto ap = state.dense_accessor<point_t, flecsi_internal>("position");

      point_t& p = ap[id()];
      vector_t& v = av[id()];

      p += v;

      if(p[0] > 1.0){
        p[0] = 0;
      }
      else if(p[0] < 0.0){
        p[0] = 1.0;
      }
      
      if(p[1] > 1.0){
        p[1] = 0;
      }
      else if(p[1] < 0.0){
        p[1] = 1.0;
      }
    }
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
      id().coordinates(range, p);
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
using vector_t = tree_topology_t::vector_t;
using branch_t = tree_topology_t::branch_t;
using branch_id_t = tree_topology_t::branch_id_t;

static const size_t N = 5000;
static const size_t TS = 2;

TEST(tree_topology, gravity) {
  tree_topology_t t;

  pseudo_random rng;

  state_t& state = state_t::instance();

  state.register_state<double, flecsi_internal>("mass", N, 0);
  state.register_state<vector_t, flecsi_internal>("velocity", N, 0);
  state.register_state<vector_t, flecsi_internal>("position", N, 0);

  vector<body*> bodies;
  for(size_t i = 0; i < N; ++i){
    double m = rng.uniform(0.1, 0.5);
    point_t p = {rng.uniform(0.0, 1.0), rng.uniform(0.0, 1.0)};
    point_t v = {rng.uniform(0.0, 0.001), rng.uniform(0.0, 0.001)};
    auto bi = t.make_entity();
    bi->init(m, p, v);
    bodies.push_back(bi);
    t.insert(bi);
  }

  size_t ix = 0;

  auto f = [&](body* b, body* b0){
    if(b0 == b){
      return;
    }
    
    b0->interact(b);
    ++ix;
  };

  auto g = 
  [&](branch_t* b, size_t depth, vector<Aggregate>& aggs) -> bool{
    if(depth > 4 || b->is_leaf()){    
      auto h = [&](body* bi, Aggregate& agg){
       agg.center += bi->mass() * bi->coordinates();
       agg.mass += bi->mass(); 
      };

      Aggregate agg;
      t.visit_children(b, h, agg);
      aggs.emplace_back(move(agg));

      return true;
    }

    return false;
  };

  for(size_t ts = 0; ts < TS; ++ts){
    //cout << "---- ts = " << ts << endl;

    vector<Aggregate> aggs;
    t.visit(t.root(), g, aggs);

    for(size_t i = 0; i < N; ++i){
      auto bi = bodies[i];
      t.apply_in_radius(bi->coordinates(), 0.01, f, bi);
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
