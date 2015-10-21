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
//! \file unstruct_types.h
//!
//! \brief This file includes the main type declarations required by the
//!        mesh_topology class.
///////////////////////////////////////////////////////////////////////////////

// include guard
#pragma once

// includes
#include "../geometry/point.h"
#include "../geometry/space_vector.h"
#include "../mesh/mesh_topology.h"
#include "../utils/common.h"

namespace flexi {

//=============================================================================
//! \class unstruct_mesh_traits_t
//!
//! \brief the main mesh traits
//!
//! You only need to specify the number of dimensions and real type
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename T, int D> struct unstruct_mesh_traits_t {

  //! \brief the number of dimensions
  static constexpr size_t dimension = D;

  //! \brief we are using double precision
  using real_t = T;
};

//=============================================================================
//! \class unstruct_mesh_types_t
//!
//! \brief A collection of the type information needed to specialize the flexi
//!        low-level mesh infastructure.
//!
//! This specialization is for a general multi-dimensional unstructured mesh.
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename T, int D> struct unstruct_mesh_types_t {

  //---------------------------------------------------------------------------
  // Public Types
  //---------------------------------------------------------------------------

  //! \brief the mesh traits
  using traits_t = unstruct_mesh_traits_t<T, D>;

  //! \brief the number of dimensions
  static constexpr auto dimension = traits_t::dimension;

  //! \brief the real type
  using real_t = typename traits_t::real_t;

  //! \brief the point type depends on dimensions and precision
  using point_t = point<real_t, dimension>;

  //! \brief a spacial vector
  using vector_t = space_vector<real_t, dimension>;

  //---------------------------------------------------------------------------
  //! \class unstruct_vertex_t
  //! \brief The unstruct_vertex_t type provides an interface for managing and
  //!        geometry and state associated with mesh vertices.
  //---------------------------------------------------------------------------
  class unstruct_vertex_t : public MeshEntity<0> {
  public:
    //! \brief Constructor with vertex
    //! \param[in] coordinates the coordinates to set
    unstruct_vertex_t(const point_t &coordinates) : coordinates_(coordinates) {}

    //! \brief set the coordinates
    //! \param[in] coordinates the coordinates to set
    void setCoordinates(const point_t &coordinates) {
      coordinates_ = coordinates;
    }

    //! \brief extract the coordinates
    const decltype(auto) &getCoordinates() const { return &coordinates_; }

  private:
    //! the coordinates of this vertex
    point_t coordinates_;
  };

  //---------------------------------------------------------------------------
  //! \class unstruct_edge_t
  //! \brief The unstruct_edge_t type provides an interface for managing and
  //!        geometry and state associated with mesh edges.
  //---------------------------------------------------------------------------
  struct unstruct_edge_t : public MeshEntity<1> {};

  //---------------------------------------------------------------------------
  //! \class unstruct_face_t unstruct_types.h
  //! \brief The unstruct_face_t type provides an interface for managing and
  //!        geometry and state associated with mesh faces.
  //---------------------------------------------------------------------------
  class unstruct_face_t : public MeshEntity<2> {};

  //---------------------------------------------------------------------------
  //! \class unstruct_cell_t unstruct_types.h
  //! \brief The unstruct_cell_t type provides an interface for managing and
  //!        geometry and state associated with mesh cells.
  //---------------------------------------------------------------------------
  class unstruct_cell_t : public MeshEntity<3> {};

  //---------------------------------------------------------------------------
  //! \brief Specify mesh parameterizations.
  //---------------------------------------------------------------------------
  using EntityTypes = std::tuple<unstruct_vertex_t, unstruct_edge_t,
      unstruct_face_t, unstruct_cell_t>;

  //---------------------------------------------------------------------------
  //! \brief figure out how many vertices per entity
  //! \param[in] dim the dimension of the entities in question
  //! \return return the number of vertices
  //---------------------------------------------------------------------------

  static size_t numVerticesPerEntity(size_t dim) {
    switch (dim) {
    case 0:
      // vertex
      return 1;
    case 1:
      // edge
      return 2;
    case 2:
      // face
      return 4;
    case 3:
      // cell
      return 8;
    default:
      assert(false && "iznvalid dimension");
    } // switch
  }
};

} // namespace flexi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
