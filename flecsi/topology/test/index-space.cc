#include <cinchtest.h>
#include <iostream>

#include "flecsi/topology/index_space.h"

using namespace std;
using namespace flecsi;
using namespace topology;

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

struct object_id{
  size_t id;

  object_id(size_t id)
  : id(id){}

  operator size_t(){
    return id;
  }

  size_t index_space_index() const{
    return id;
  }

  bool operator<(const object_id& oid) const{
    return id < oid.id;
  }
};

struct object{
  object(object_id id)
  : id(id){}

  using id_t = object_id;

  object_id index_space_id() const{
    return id;
  }

  object_id id;

  double mass;
};

TEST(index_space, index_space) {
  using index_space_t = index_space<object*, true, true, false>;

  index_space_t is;

  for(size_t i = 0; i < 10000; ++i){
    is << new object(i);
    is[i]->mass = uniform(0.0, 1.0);
  }

  forall(is, o,
    cout << o->id << endl;
  );

  double total_mass = 0;

  reduce_all(is, o, total_mass,
    total_mass += o->mass;
  );

  cout << "total_mass: " << total_mass << endl;
}
