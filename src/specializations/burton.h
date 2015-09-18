/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_burton_h
#define jali_burton_h

#include "../base/mesh_topology.h"

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace jali {

/*!
  \class burton burton.h
  \brief burton provides...
 */
class burton
{
public:

  // Vertex type
  struct burton_vertex_t : public MeshEntity {
    burton_vertex_t(size_t id) : MeshEntity(id) {}
  }; // struct burton_vertex_t

  // Edge type
  struct burton_edge_t : public MeshEntity {
    burton_edge_t(size_t id) : MeshEntity(id) {}
  }; // struct burton_edge_t

  // Side type
  struct burton_side_t : public MeshEntity {
    burton_side_t(size_t id) : MeshEntity(id) {}
  }; // struct burton_side_t

  // Cell type
  class burton_cell_t : public MeshEntity {
    public:

      burton_cell_t(size_t id) : MeshEntity(id) {}

      EntityGroup<2> & getSides() {
        return sides_;
      } // getSides

    private:

      EntityGroup<2> sides_;

  }; // struct burton_cell_t

  using EntityTypes =
    std::tuple<burton_vertex_t, burton_edge_t, burton_cell_t>;

  //! Default constructor
  burton() {}

  //! Copy constructor (disabled)
  burton(const burton &) = delete;

  //! Assignment operator (disabled)
  burton & operator = (const burton &) = delete;

  //! Destructor
  ~burton() {}

private:

}; // class burton

} // namespace jali

#endif // jali_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
