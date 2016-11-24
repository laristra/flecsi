/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_simple_definition_h
#define flecsi_io_simple_definition_h

#include <fstream>
#include <unordered_map>
#include <string>

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
    file_.open(filename, std::ifstream::in);

    if(file_.good()) {
      std::string line;
      std::getline(file_, line);
      std::istringstream iss(line);

      // Read the number of vertices and cells
      iss >> num_vertices_ >> num_cells_;

      // Get the offset to the beginning of the vertices
      vertex_start_ = file_.tellg();

      std::getline(file_, line);

      vertex_size_ = file_.tellg() - vertex_start_;

      for(size_t i(0); i<num_vertices_-1; ++i) {
        std::getline(file_, line);
      } // for

      cell_start_ = file_.tellg();

      std::getline(file_, line);

      cell_size_ = file_.tellg() - cell_start_;
  } // if

  }

  /// Copy constructor (disabled)
  simple_definition_t(const simple_definition_t &) = delete;

  /// Assignment operator (disabled)
  simple_definition_t & operator = (const simple_definition_t &) = delete;

  /// Destructor
   ~simple_definition_t() {}

  size_t dimension() { return 2; }
  size_t num_vertices() { return num_vertices_; }
  size_t num_cells() { return num_cells_; }

  ///
  //
  ///
  std::vector<size_t>
  vertices( 
    size_t cell_id
  )
  {
    std::string line;
    std::vector<size_t> ids;
    size_t v0, v1, v2, v3;

    file_.seekg(cell_start_ +
      static_cast<std::iostream::pos_type>(cell_id)*cell_size_);
    std::getline(file_, line);
    std::istringstream iss(line);

    iss >> v0 >> v1 >> v2 >> v3;

    ids.push_back(v0);
    ids.push_back(v1);
    ids.push_back(v2);
    ids.push_back(v3);

    return ids;
  } // vertices

  ///
  //
  ///
  point_t
  vertex(
    size_t vertex_id
  )
  {
    std::string line;
    point_t v;

    file_.seekg(vertex_start_ +
      static_cast<std::iostream::pos_type>(vertex_id)*vertex_size_);
    std::getline(file_, line);
    std::istringstream iss(line);

    iss >> v[0] >> v[1];

    return v;
  } // vertex

private:

  std::ifstream file_;

  size_t num_vertices_;
  size_t num_cells_;

  std::iostream::pos_type vertex_start_;
  std::iostream::pos_type cell_start_;

  std::iostream::pos_type vertex_size_;
  std::iostream::pos_type cell_size_;

}; // class simple_definition_t

} // namespace io
} // namespace flecsi

#endif // flecsi_io_simple_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
