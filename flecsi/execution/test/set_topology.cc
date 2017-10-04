/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#include <array>

#include <cinchtest.h>

#include "flecsi/topology/set_topology.h"
#include "flecsi/topology/types.h"
#include "flecsi/coloring/coloring_types.h"

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace coloring;

class entity1 : public set_entity_t{

};

class entity2 : public set_entity_t{

};

class set_types{
  using entity_types = std::tuple<
    std::tuple<index_space_<0>, entity1>,
    std::tuple<index_space_<1>, entity2>
    >;
};

using set_topology = set_topology_t<set_types>;

namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char ** argv) {
  context_t & context_ = context_t::instance();
  
  size_t n = 100;
  size_t colors = context_.colors();
  size_t nc = n/colors;

  std::unordered_map<size_t, coloring_info_t> coloring_info;
  
  for(size_t i = 0; i < colors; ++i){
    coloring_info_t ci;
    ci.exclusive = nc;
    ci.shared = 0;
    ci.ghost = 0;

    coloring_info[i] = std::move(ci);
  }

  context_.add_set_coloring(0, coloring_info);
}

void specialization_spmd_init(int argc, char ** argv) {

}

void driver(int argc, char ** argv) {
}

} // namespace execution
} // namespace flecsi

TEST(set_topology, test1) {
  
} // TEST