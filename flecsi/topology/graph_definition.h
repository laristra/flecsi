/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_graph_definition_h
#define flecsi_topology_graph_definition_h

#include <set>
#include <vector>

#include "flecsi/geometry/point.h"

///
/// \file
/// \date Initial file creation: Nov 17, 2016
///

namespace flecsi {
namespace topology {

///
/// \class graph_definition__ graph_definition.h
/// \brief graph_definition__ provides...
///
template<size_t D>
class graph_definition__
{
public:

  using point_t = point<double, D>;

  /// Default constructor
  graph_definition__() {}

  /// Copy constructor (disabled)
  graph_definition__(const graph_definition__ &) = delete;

  /// Assignment operator (disabled)
  graph_definition__ & operator = (const graph_definition__ &) = delete;

  /// Destructor
  virtual ~graph_definition__() {}
  
  ///
  /// Return the dimension of the graph.
  ///
  constexpr
  size_t
  dimension()
  const
  {
    return D;
  } // dimension

  virtual size_t num_entities(size_t dimension) = 0;

  virtual std::vector<size_t> vertices(size_t dimension, size_t entity_id) = 0;
  virtual std::set<size_t> vertex_set(size_t dimension, size_t entity_id) = 0;

  virtual point_t vertex(size_t vertex_id) = 0;

private:

}; // class graph_definition__

using graph_definition_t = graph_definition__<2>;

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_graph_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
