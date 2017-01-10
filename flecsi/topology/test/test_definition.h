/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_test_definition_h
#define flecsi_topology_test_definition_h

#include "flecsi/geometry/point.h"
#include "flecsi/utils/logging.h"

///
// \file test_definition.h
// \authors bergen
// \date Initial file creation: Dec 11, 2016
///

namespace flecsi {
namespace topology {

///
// \class test_definition_t test_definition.h
// \brief test_definition_t provides...
///
class test_definition_t
  : public graph_definition_t
{
public:

  double vertices_[25][3] = {
    { 0.0,  0.0,  0.0 },
    { 0.25, 0.0,  0.0 },
    { 0.5,  0.0,  0.0 },
    { 0.75, 0.0,  0.0 },
    { 1.0,  0.0,  0.0 },
    { 0.0,  0.25, 0.0 },
    { 0.25, 0.25, 0.0 },
    { 0.5,  0.25, 0.0 },
    { 0.75, 0.25, 0.0 },
    { 1.0,  0.25, 0.0 },
    { 0.0,  0.5,  0.0 },
    { 0.25, 0.5,  0.0 },
    { 0.5,  0.5,  0.0 },
    { 0.75, 0.5,  0.0 },
    { 1.0,  0.5,  0.0 },
    { 0.0,  0.75, 0.0 },
    { 0.25, 0.75, 0.0 },
    { 0.5,  0.75, 0.0 },
    { 0.75, 0.75, 0.0 },
    { 1.0,  0.75, 0.0 },
    { 0.0,  1.0,  0.0 },
    { 0.25, 1.0,  0.0 },
    { 0.5,  1.0,  0.0 },
    { 0.75, 1.0,  0.0 },
    { 1.0,  1.0,  0.0 }
  }; // vertices_

  size_t cells_[16][4] = {
    { 0,  1,  5,  6  },
    { 1,  2,  6,  7  },
    { 2,  3,  7,  8  },
    { 3,  4,  8,  9  },
    { 5,  6,  10, 11 },
    { 6,  7,  11, 12 },
    { 7,  8,  12, 13 },
    { 8,  9,  13, 14 },
    { 10, 11, 15, 16 },
    { 11, 12, 16, 17 },
    { 12, 13, 17, 18 },
    { 13, 14, 18, 19 },
    { 15, 16, 20, 21 },
    { 16, 17, 21, 22 },
    { 17, 18, 22, 23 },
    { 18, 19, 23, 24 }
  }; // cells_

  /// Default constructor
  test_definition_t() {}

  /// Copy constructor (disabled)
  test_definition_t(const test_definition_t &) = delete;

  /// Assignment operator (disabled)
  test_definition_t & operator = (const test_definition_t &) = delete;

  /// Destructor
  ~test_definition_t() {}

  size_t
  num_entities(
    size_t topological_dimension
  ) override
  {
    return topological_dimension == dimension() ? 16 :
      topological_dimension == 0 ? 25 :
      clog_fatal("invalid topological dimension");
  } // num_entities

  std::vector<size_t>
  vertices(
    size_t topological_dimension,
    size_t entity_id
  ) override
  {
    std::vector<size_t> ids(4);

    for(size_t i(0); i<4; ++i) {
      ids[i] = cells_[entity_id][i]; 
    } // for

    return ids;
  } // vertices

  std::set<size_t>
  vertex_set(
    size_t topological_dimension,
    size_t entity_id
  ) override
  {
    auto vvec = vertices(topological_dimension, entity_id);
    return std::set<size_t>(vvec.begin(), vvec.end());
  } // vertex_set

  point_t
  vertex(
    size_t vertex_id
  ) override
  {
    return point_t(vertices_[vertex_id][0], vertices_[vertex_id][1]);
  } // vertex

private:

}; // class test_definition_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_test_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
