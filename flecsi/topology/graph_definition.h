/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_graph_definition_h
#define flecsi_topology_graph_definition_h

#include "flecsi/geometry/point.h"

///
// \file graph_definition.h
// \authors bergen
// \date Initial file creation: Nov 17, 2016
///

namespace flecsi {
namespace topology {

///
// \class graph_definition_t graph_definition.h
// \brief graph_definition_t provides...
///
class graph_definition_t
{
public:

  using point_t = point<double, 3>;

  /// Default constructor
  graph_definition_t() {}

  /// Copy constructor (disabled)
  graph_definition_t(const graph_definition_t &) = delete;

  /// Assignment operator (disabled)
  graph_definition_t & operator = (const graph_definition_t &) = delete;

  /// Destructor
  virtual ~graph_definition_t() {}

  virtual size_t dimension() = 0;

  virtual size_t num_entities(size_t dimension) = 0;

  virtual std::vector<size_t> vertices(size_t dimension, size_t entity_id) = 0;
  virtual std::set<size_t> vertex_set(size_t dimension, size_t entity_id) = 0;

  virtual point_t vertex(size_t vertex_id) = 0;

private:

}; // class graph_definition_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_graph_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
