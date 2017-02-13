#include <cinchtest.h>
#include <iostream>

//#include "flecsi/topology/index_space.h"
#include "flecsi/execution/execution.h"

#include "pseudo_random.h"

using namespace std;
using namespace flecsi;
using namespace topology;


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

  double mass = 0.0;
  int tag = 0;
};

TEST(index_space, index_space) {

  pseudo_random rng;

  using index_space_t = index_space<object*, true, true, false>;
  index_space_t is;

  for(size_t i = 0; i < 10000; ++i){
    is << new object(i);
    is[i]->mass = rng.uniform(0.0, 1.0);
  }

  size_t cnt = 0;
  flecsi_for_each(o, is, {
    // std::cout << o->id << endl;
    ASSERT_EQ( o->index_space_id().index_space_index(), cnt++ );
  }); // foreach

	double total_mass(0.0);

	flecsi_reduce_each(o, is, total_mass, {
   	total_mass += o->mass;
	});

  std::cout << "total_mass: " << total_mass << std::endl;
  int mass_check = total_mass * 100;
  ASSERT_EQ( mass_check, 500342 );

#if 0
  forall(is, o,
    cout << o->id << endl;
  );

  double total_mass = 0;

  reduce_all(is, o, total_mass,
    total_mass += o->mass;
  );

  cout << "total_mass: " << total_mass << endl;
#endif
}


TEST(index_space, bin) {

  // create an index space object for testing
  using index_space_t = index_space<object, true, true, false>;
  index_space_t is;

  constexpr size_t num_objects = 10;

  // initialize it
  for(size_t i = 0; i < num_objects; ++i){
    is.push_back( object(i) );
    is[i].tag = (i<num_objects/2) ? 0 : 1;
  }

  // bin the index space into two seperate bins based on the
  // tag
  auto bins = is.bin_as_vector( [](const auto & o) { return o.tag; } );
 
  // there should be two bins with 500 elements each.
  ASSERT_EQ( bins.size(), 2 );
  ASSERT_EQ( bins.at(0).size(), num_objects/2 );
  ASSERT_EQ( bins.at(1).size(), num_objects/2 );

  // make sure what is in the bins is correct.
  for(size_t i = 0; i < num_objects; ++i){
    if ( i < num_objects/2 ) {
      ASSERT_EQ( bins.at(0)[i].id.index_space_index(), i );
      ASSERT_EQ( bins.at(0)[i].tag, 0 );
    }
    else {
      auto j = i - num_objects/2;
      ASSERT_EQ( bins.at(1)[j].id.index_space_index(), i );
      ASSERT_EQ( bins.at(1)[j].tag, 1 );
    }
  }
}
