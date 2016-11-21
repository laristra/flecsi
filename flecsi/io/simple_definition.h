/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_simple_definition_h
#define flecsi_io_simple_definition_h

#include <ifstream>

#include "flecsi/io/mesh_definition.h"

///
// \file simple_definition.h
// \authors bergen
// \date Initial file creation: Nov 21, 2016
///

namespace flecsi {
namespace io {

///
// \class simple_definition_t simple_definition.h
// \brief simple_definition_t provides...
///
class simple_definition_t
  : public mesh_definition_t
{
public:

  /// Default constructor
  simple_definition_t(
    const char * filename
  )
  {
  }

  /// Copy constructor (disabled)
  simple_definition_t(const simple_definition_t &) = delete;

  /// Assignment operator (disabled)
  simple_definition_t & operator = (const simple_definition_t &) = delete;

  /// Destructor
   ~simple_definition_t() {}

  std::vector<size_t>
  vertices( 
    size_t cell_id
  )
  {
  } // vertices

  std::tuple<double, double, double>
  vertex(
    size_t vertex_id
  )
  {
  } // vertex

private:

}; // class simple_definition_t

} // namespace io
} // namespace flecsi

#endif // flecsi_io_simple_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
