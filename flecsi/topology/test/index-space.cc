#include <cinchtest.h>
#include <iostream>

//#include "flecsi/topology/index_space.h"
#include "flecsi/execution/execution.h"

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
};

TEST(index_space, index_space) {
  using index_space_t = index_space<object*, true, true, false>;

  index_space_t is;

  for(size_t i = 0; i < 10000; ++i){
    is << new object(i);
  }

  for_each(is, o, {
    cout << o->id << endl;
  }); // foreach
}
