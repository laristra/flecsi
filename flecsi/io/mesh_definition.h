/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_mesh_definition_h
#define flecsi_io_mesh_definition_h

#include <set>
#include <vector>

#include "flecsi/geometry/point.h"

///
/// \file
/// \date Initial file creation: Nov 17, 2016
///

namespace flecsi {
namespace io {

///
/// \class mesh_definition_t mesh_definition.h
/// \brief mesh_definition_t provides...
///
class mesh_definition_t
{
public:

  using point_t = point<double, 2>;

  /// Default constructor
  mesh_definition_t() {}

  /// Copy constructor (disabled)
  mesh_definition_t(const mesh_definition_t &) = delete;

  /// Assignment operator (disabled)
  mesh_definition_t & operator = (const mesh_definition_t &) = delete;

  /// Destructor
  virtual ~mesh_definition_t() {}

  virtual size_t dimension() = 0;
  virtual size_t num_vertices() = 0;
  virtual size_t num_cells() = 0;

  virtual std::vector<size_t> vertices(size_t cell_id) = 0;
  virtual std::set<size_t> vertices_set(size_t cell_id) = 0;

  virtual point_t vertex(size_t vertex_id) = 0;

private:

}; // class mesh_definition_t

} // namespace io
} // namespace flecsi

#endif // flecsi_io_mesh_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
