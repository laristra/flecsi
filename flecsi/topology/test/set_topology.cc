/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#include <array>

#include "flecsi/topology/set_topology.h"
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/topology/mesh_types.h"

using namespace std;
using namespace flecsi;
using namespace topology;

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
