/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_mesh_definition_h
#define flecsi_io_mesh_definition_h

///
// \file mesh_definition.h
// \authors bergen
// \date Initial file creation: Nov 17, 2016
///

namespace flecsi {
namespace io {

///
// \class mesh_definition_t mesh_definition.h
// \brief mesh_definition_t provides...
///
class mesh_definition_t
{
public:

  /// Default constructor
  mesh_definition_t() {}

  /// Copy constructor (disabled)
  mesh_definition_t(const mesh_definition_t &) = delete;

  /// Assignment operator (disabled)
  mesh_definition_t & operator = (const mesh_definition_t &) = delete;

  /// Destructor
  virtual ~mesh_definition_t() {}

  virtual std::vector<size_t> vertices(size_t cell_id) = 0;

  virtual std::tuple<double, double, double> vertex(size_t vertex_id) = 0;

private:

}; // class mesh_definition_t

} // namespace io
} // namespace flecsi

#endif // flecsi_io_mesh_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
