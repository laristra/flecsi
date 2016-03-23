/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
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
#include "../../execution/task.h"
#include "../../mesh/mesh_topology.h"
#include "unstruct_types.h"

namespace flecsi
{
//=============================================================================
//! \class unstruct_mesh_t
//!
//! \brief provides the actual implementation of the unstructured mesh
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename R, int D>
class unstruct_mesh_t
{
  //---------------------------------------------------------------------------
  // Private Types
  //---------------------------------------------------------------------------

 private:
  //! \brief the mesh types tuple
  using mesh_types_t = unstruct_mesh_types_t<R, D>;

  //! \brief the mesh traits type
  using traits_t = unstruct_mesh_traits_t<R, D>;

  //! \brief the mesh topology type
  using mesh_topology_t = mesh_topology<mesh_types_t>;

  //---------------------------------------------------------------------------
  // Intermediate API
  //---------------------------------------------------------------------------

 public:
#ifndef MESH_EXECUTION_POLICY
  // for now: use default execution policy
  using mesh_execution_t = execution_t<>;
#else
  using mesh_execution_t = execution_t<MESH_STORAGE_POLICY>;
#endif

  //! \brief Type defining the data attachment sites on the mesh.
  using attachment_site_t = typename traits_t::attachment_site_t;

  //! \brief Accessor type.
  template <typename T>
  using accessor_t = typename traits_t::mesh_state_t::template accessor_t<T>;

  //---------------------------------------------------------------------------
  //! \brief Register state for the named variable at the given attachment
  //! site.
  //!
  //! \param key A const_string_t name for the state variable, e.g., "density".
  //! \param site The data attachement site where the state variable should
  //!   be defined.  Valid sites are defined in flecsi::burton_mesh_traits_t.
  //! \param attributes A bitfield specifying various attributes of the state.
  //!
  //! \return An accessor to the newly registered state.
  //---------------------------------------------------------------------------
  template <typename T>
  decltype(auto) register_state_(const const_string_t && key,
      attachment_site_t site, bitfield_t::field_type_t attributes = 0x0)
  {
    switch (site) {
      case attachment_site_t::vertices:
        return state_.template register_state<T>(
            key, num_vertices(), attachment_site_t::vertices, attributes);
        break;
      case attachment_site_t::edges:
        return state_.template register_state<T>(
            key, num_edges(), attachment_site_t::edges, attributes);
        break;
      case attachment_site_t::cells:
        return state_.template register_state<T>(
            key, num_cells(), attachment_site_t::cells, attributes);
        break;
      default:
        assert(false && "Error: invalid state registration site.");
    } // switch

  } // register_state_

  //! FIXME
  template <typename T>
  auto access_state_(const const_string_t && key)
  {
    return state_.template accessor<T>(key);
  }

  //! FIXME
  template <typename T>
  auto access_type_()
  {
    return state_.template accessors<T>();
  }

  //! FIXME
  template <typename T, typename P>
  auto access_type_if_(P && predicate)
  {
    return state_.template accessors<T, P>(std::forward<P>(predicate));
  }

  //! FIXME
  auto state_attributes_(const const_string_t && key)
  {
    return state_.template meta_data<>((key)).attributes;
  }

  //---------------------------------------------------------------------------
  // Public Types
  //---------------------------------------------------------------------------

  //! \brief the number of dimensions
  static constexpr auto dimension = traits_t::dimension;

  //! \brief the real type
  using real_t = typename traits_t::real_t;

  //! \brief the point type depends on dimensions and precision
  using point_t = point<real_t, dimension>;

  //! \brief the vertex type
  using vertex_t = typename mesh_types_t::vertex_t;
  //! \brief the edge type
  using edge_t = typename mesh_types_t::edge_t;
  //! \breif the face type
  using face_t = typename mesh_types_t::face_t;
  //! \brief the cell type
  using cell_t = typename mesh_types_t::cell_t;

  //---------------------------------------------------------------------------
  // Deleted member functions
  //---------------------------------------------------------------------------

  //! \brief Copy constructor (disabled)
  unstruct_mesh_t(const unstruct_mesh_t &) = delete;

  //! \brief Assignment operator (disabled)
  unstruct_mesh_t & operator=(const unstruct_mesh_t &) = delete;

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
  auto vertices() { return mesh_.vertices<0>(); }
  auto vertex_ids() { return mesh_.vertex_ids<0>(); }
  template <class E>
  auto vertices(E * e)
  {
    return mesh_.vertices<0>(e);
  }
  template <class E>
  auto vertex_ids(E * e)
  {
    return mesh_.vertex_ids<0>(e);
  }

  auto num_vertices() const { return mesh_.template num_vertices<0>(); }
  //---------------------------------------------------------------------------
  //! \brief get all the edges
  //! \return a pointer to the list of edges
  //---------------------------------------------------------------------------
  auto edges() { return mesh_.edges<0>(); }
  auto edge_ids() { return mesh_.edge_ids<0>(); }
  template <class E>
  auto edges(E * e)
  {
    return mesh_.edges<0>(e);
  }
  template <class E>
  auto edge_ids(E * e)
  {
    return mesh_.edge_ids<0>(e);
  }

  auto num_edges() const { return mesh_.template num_edges<0>(); }
  //---------------------------------------------------------------------------
  //! \brief get all the faces
  //! \return a pointer to the list of faces
  //---------------------------------------------------------------------------
  auto faces() { return mesh_.faces<0>(); }
  auto face_ids() { return mesh_.face_ids<0>(); }
  template <class E>
  auto faces(E * e)
  {
    return mesh_.faces<0>(e);
  }
  template <class E>
  auto face_ids(E * e)
  {
    return mesh_.face_ids<0>(e);
  }

  auto num_faces() const { return mesh_.template num_faces<0>(); }
  //---------------------------------------------------------------------------
  //! \brief get all the cells
  //! \return a pointer to the list of cells
  //---------------------------------------------------------------------------
  auto cells() { return mesh_.cells<0>(); }
  auto cell_ids() { return mesh_.cell_ids<0>(); }
  template <class E>
  auto cells(E * e)
  {
    return mesh_.cells<0>(e);
  }
  template <class E>
  auto cell_ids(E * e)
  {
    return mesh_.cell_ids<0>(e);
  }

  auto num_cells() const { return mesh_.template num_cells<0>(); }
  //---------------------------------------------------------------------------
  //! \brief create a vertex
  //! \param[in] pos the coordinates to the vertex
  //! \return a pointer to the vertex
  //---------------------------------------------------------------------------
  auto create_vertex(const point_t & pos)
  {
    auto p = access_state_<point_t>("coordinates");
    p[mesh_.num_vertices()] = pos;

    auto v = mesh_.template make<vertex_t, 0>(pos, &state_);
    mesh_.template add_vertex<0>(v);
    return v;
  }

  //---------------------------------------------------------------------------
  //! \brief create a cell
  //! \param[in] verts the list of pointers to cell vertices
  //! \return a pointer to the cell
  //---------------------------------------------------------------------------
  auto create_cell(std::initializer_list<vertex_t *> verts)
  {
    auto c = mesh_.template make<cell_t, 0>();
    mesh_.template init_cell<0>(c, verts);
    return c;
  }

  //---------------------------------------------------------------------------
  //! \brief initialize some data that requires state storage
  //! \param[in] vertices the number of total vertices
  //---------------------------------------------------------------------------
  void init_parameters(size_t vertices)
  {
    // register coordinate state
    state_.template register_state<point_t>(
        "coordinates", vertices, attachment_site_t::vertices, persistent);
  }

  //---------------------------------------------------------------------------
  //! \brief the main init function
  //---------------------------------------------------------------------------
  void init() { mesh_.template init<0>(); }
  //---------------------------------------------------------------------------
  // Member data
  //---------------------------------------------------------------------------

 private:
  //! the actual mesh object
  mesh_topology_t mesh_;

  //! the state data
  typename traits_t::mesh_state_t state_;
};

//! some aliases for 2 and 3 dimensional mesh
using unstruct_2d_mesh_t = unstruct_mesh_t<double, 2>;
using unstruct_3d_mesh_t = unstruct_mesh_t<double, 3>;

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
