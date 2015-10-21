/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

///////////////////////////////////////////////////////////////////////////////
//! \file types.h
//!
//! \brief This file includes the main declaration of the general
//!        unstructured mesh.
///////////////////////////////////////////////////////////////////////////////

// include guard
#pragma once

// includes
#include "unstruct_types.h"

namespace flexi {

//=============================================================================
//! \class unstruct_mesh_t
//!
//! \brief provides the actual implementation of the unstructured mesh
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename T, int D> class unstruct_mesh_t {

  //---------------------------------------------------------------------------
  // Types
  //---------------------------------------------------------------------------

private:
  //! \brief the mesh types tuple
  using mesh_types_t = unstruct_mesh_types_t<T, D>;

  //! \brief the mesh traits type
  using traits_t = unstruct_mesh_traits_t<T, D>;

  //! \brief the mesh topology type
  using mesh_topology_t = MeshTopology<mesh_types_t>;

public:
  //! \brief the number of dimensions
  static constexpr auto dimension = traits_t::dimension;

  //! \brief the real type
  using real_t = typename traits_t::real_t;

  //! \brief the point type depends on dimensions and precision
  using point_t = point<real_t, dimension>;

  //! \brief the vertex type
  using vertex_t = typename mesh_types_t::unstruct_vertex_t;
  //! \brief the edge type
  using edge_t = typename mesh_types_t::unstruct_edge_t;
  //! \breif the face type
  using face_t = typename mesh_types_t::unstruct_face_t;
  //! \brief the cell type
  using cell_t = typename mesh_types_t::unstruct_cell_t;

  //! \brief The vertex iterator types
  using vertex_iterator_t = typename mesh_topology_t::VertexIterator;
  //! \brief The edge iterator types
  using edge_iterator_t = typename mesh_topology_t::EdgeIterator;
  //! \brief The face iterator types
  using face_iterator_t = typename mesh_topology_t::FaceIterator;
  //! \brief The cell iterator types
  using cell_iterator_t = typename mesh_topology_t::CellIterator;

  //---------------------------------------------------------------------------
  // Deleted member functions
  //---------------------------------------------------------------------------

  //! \brief Copy constructor (disabled)
  unstruct_mesh_t(const unstruct_mesh_t &) = delete;

  //! \brief Assignment operator (disabled)
  unstruct_mesh_t &operator=(const unstruct_mesh_t &) = delete;

  //---------------------------------------------------------------------------
  //! \brief Default member functions
  //---------------------------------------------------------------------------
  unstruct_mesh_t() {}

  //---------------------------------------------------------------------------
  //! \brief Destructor
  //---------------------------------------------------------------------------
  ~unstruct_mesh_t() {}

  //---------------------------------------------------------------------------
  //! \brief get the vertices
  //! \return a pointer to the list of vertices
  //---------------------------------------------------------------------------
  auto vertices() { return mesh_.vertices(); }

  //---------------------------------------------------------------------------
  //! \brief get all the edges
  //! \return a pointer to the list of edges
  //---------------------------------------------------------------------------
  auto edges() { return mesh_.edges(); }

  //---------------------------------------------------------------------------
  //! \brief get all the faces
  //! \return a pointer to the list of faces
  //---------------------------------------------------------------------------
  auto faces() { return mesh_.faces(); }

  //---------------------------------------------------------------------------
  //! \brief get all the cells
  //! \return a pointer to the list of cells
  //---------------------------------------------------------------------------
  auto cells() { return mesh_.cells(); }

  //---------------------------------------------------------------------------
  //! \brief create a vertex
  //! \param[in] pos the coordinates to the vertex
  //! \return a pointer to the vertex
  //---------------------------------------------------------------------------
  auto create_vertex(const point_t &pos) {
    auto v = mesh_.template make<vertex_t>(pos);
    mesh_.addVertex(v);
    return v;
  }

  //---------------------------------------------------------------------------
  //! \brief create a cell
  //! \param[in] verts the list of pointers to cell vertices
  //! \return a pointer to the cell
  //---------------------------------------------------------------------------
  auto create_cell(std::initializer_list<vertex_t *> verts) {
    auto c = mesh_.template make<cell_t>();
    mesh_.addCell(c, verts);
    return c;
  }

  //---------------------------------------------------------------------------
  //! \brief the main init function
  //---------------------------------------------------------------------------
  void init() {}

  //---------------------------------------------------------------------------
  // Member data
  //---------------------------------------------------------------------------

private:
  //! the actual mesh object
  mesh_topology_t mesh_;
};

} // namespace flexi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
