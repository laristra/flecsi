/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_burton_entity_types_h
#define flexi_burton_entity_types_h

#include "burton_mesh_traits.h"

/*!
 * \file burton_entity_types.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flexi {

/*----------------------------------------------------------------------------*
 * class burton_vertex_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_vertex_t burton_entity_types.h
  \brief The burton_vertex_t type provides an interface for managing
    geometry and state associated with mesh vertices.
  \tparam size_t N number of domains
 */
template<size_t N>
class burton_vertex_t : public mesh_entity_t<0, N>
{
public:

  //! Type containing coordinates of the vertex
  using point_t = burton_mesh_traits_t::point_t;
  //! Handle for accessing state at vertex
  using state_t = burton_mesh_traits_t::mesh_state_t;
  //! Number of domains in the burton mesh
  static constexpr size_t num_domains = burton_mesh_traits_t::num_domains;

  //! Constructor
  burton_vertex_t(state_t & state)
      : precedence_(0), state_(state) {}

  /*!
    \brief Set the rank for the vertex
    \tparam M FIXME
    \param uint8_t rank to give the vertex
   */
  template<size_t M>
  void set_rank(uint8_t rank) {
    mesh_entity_t<0, N>::template set_info<M>(rank);
  }

  /*!
    \brief Get the precedence for the vertex
    \tparam M FIXME
   */
  template<size_t M>
  uint64_t precedence() const {
    return 1 << (63 - mesh_entity_t<0, N>::template info<M>());
  } // precedence

  /*!
    \brief Set the coordinates for a vertex
    \param point_t coordinates value to set at vertex
   */
  void set_coordinates(const point_t &coordinates) {
    auto c = state_.accessor<point_t,flexi_internal>("coordinates");
    c[mesh_entity_base_t<num_domains>::template id<0>()] = coordinates;
  } // set_coordinates

  /*!
    \brief Get the coordinates at a vertex from the state handle
   */
  const point_t & coordinates() const {
    auto c = state_.accessor<point_t,flexi_internal>("coordinates");
    return c[mesh_entity_base_t<num_domains>::template id<0>()];
  } // coordinates

private:

  uint64_t precedence_;
  state_t & state_;

}; // class burton_vertex_t

/*----------------------------------------------------------------------------*
 * class burton_edge_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_edge_t burton_entity_types.h
  \brief The burton_edge_t type provides an interface for managing
    geometry and state associated with mesh edges.
  \tparam size_t N the domain of the edge_t
 */
template<size_t N>
struct burton_edge_t : public mesh_entity_t<1, N> {}; // struct burton_edge_t

template<size_t N>
class burton_corner_t; // class burton_corner_t

template<size_t N>
class burton_wedge_t; // class burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_cell_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_cell_t burton_entity_types.h
  \brief The burton_cell_t type provides an interface for managing and
    geometry and state associated with mesh cells.
  \tparam size_t N The domain in which to create the entity.
 */
template<size_t N>
struct burton_cell_t : public mesh_entity_t<2, N> {

  //! Constructor
  burton_cell_t() {}
  //! Destructor
  virtual ~burton_cell_t() {}

  /*!
    \brief create_entities is a function that creates entities
      of topological dimension dim, using vertices v, and puts the vertices
      in e. See, e.g., burton_quadrilateral_cell_t for an implementation of
      this pure virtual function.
    \param size_t dim The topological dimension of the entity to create.
    \param std::vector<id_t> &e Vector to fill with ids of the vertices making
      the entity.
    \param id_t * v vertex IDs.
    \param size_t vertex_count The number of vertices making up the entity.
    \return std::pair<size_t, std::vector<id_t>> A pair with the first entry
      the number of vertex collections making up the entity, and the second
      entry the number of vertices per collection.
   */
  virtual std::pair<size_t, std::vector<id_t>> create_entities(size_t dim,
    std::vector<id_t> &e, id_t * v, size_t vertex_count) = 0;

  /*!
    \brief create_bound_entities binds mesh entities across domains.
      See, e.g., burton_quadrilateral_cell_t for an implementation of
      this pure virtual function.
    \param size_t dim The topological dimension of the entity being bound.
    \param const std::vector<std::vector<id_t>> ent_ids The entity ids
      of the entities making up the binding.
    \param std::vector<id_t> & c The collection of the ids making up the bound
      entity.
    \return std::pair<size_t, std::vector<id_t>> A pair with the first entry
      the number of entity collections making up the binding, and the second
      entry the number of entities per collection.
   */
  virtual std::pair<size_t, std::vector<id_t>> create_bound_entities(
    size_t dim, const std::vector<std::vector<id_t>> ent_ids,
    std::vector<id_t> & c) = 0;

}; // class burton_cell_t

/*----------------------------------------------------------------------------*
 * class burton_quadrilateral_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_quadrilateral_t burton_entity_types.h
  \brief The burton_quadrilateral_t type provides a derived instance of
    burton_cell_t for 2D quadrilateral cells.
 */
template<size_t N>
class burton_quadrilateral_cell_t : public burton_cell_t<N>
{
public:

  /*!
    \brief create_entities function for burton_quadrilateral_cell_t.
   */
  std::pair<size_t, std::vector<id_t>> create_entities(size_t dim,
    std::vector<id_t> & e, id_t * v, size_t vertex_count) {
    
    e.resize(8);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[2];
    e[5] = v[3];

    e[6] = v[3];
    e[7] = v[0];

    return {4, {2, 2, 2, 2}};
  } // create_entities
  
  /*!
    \brief create_bound_entities function for burton_quadrilateral_cell_t.
   */
  std::pair<size_t, std::vector<id_t>> create_bound_entities(size_t dim,
    const std::vector<std::vector<id_t>> ent_ids, std::vector<id_t> & c) {

    switch(dim) {
      // Corners
      case 1:
        c.resize(8);

        // corner 0
        c[0] = ent_ids[0][0]; // vertex 0
        c[1] = ent_ids[2][0]; // cell

        // corner 1
        c[2] = ent_ids[0][1]; // vertex 1
        c[3] = ent_ids[2][0]; // cell

        // corner 2
        c[4] = ent_ids[0][2]; // vertex 2
        c[5] = ent_ids[2][0]; // cell

        // corner 3
        c[6] = ent_ids[0][3]; // vertex 3
        c[7] = ent_ids[2][0]; // cell

        return {4, {2, 2, 2, 2}};

#if 0 // Wedges are currently only referenced through corners
      // so this logic is unused for the time being...

      // Wedges
      case 2:
        c.resize(16);

        // wedge 0
        c[0] = ent_ids[0]; // vertex 0
        c[1] = ent_ids[7]; // edge 3

        // wedge 1
        c[2] = ent_ids[0]; // vertex 0
        c[3] = ent_ids[4]; // edge 0

        // wedge 2
        c[4] = ent_ids[1]; // vertex 1
        c[5] = ent_ids[4]; // edge 0

        // wedge 3
        c[6] = ent_ids[1]; // vertex 1
        c[7] = ent_ids[5]; // edge 1

        // wedge 4
        c[8] = ent_ids[2]; // vertex 2
        c[9] = ent_ids[5]; // edge 1

        // wedge 5
        c[10] = ent_ids[2]; // vertex 2
        c[11] = ent_ids[6]; // edge 2

        // wedge 6
        c[12] = ent_ids[3]; // vertex 3
        c[13] = ent_ids[6]; // edge 2

        // wedge 7
        c[14] = ent_ids[3]; // vertex 3
        c[15] = ent_ids[7]; // edge 3

        return {8, {2, 2, 2, 2, 2, 2, 2, 2}};
#endif

      default:
        assert(false && "Unknown bound entity type");
    } // switch
  } // create_bound_entities

}; // class burton_quadrilateral_cell_t

/*----------------------------------------------------------------------------*
 * class burton_wedge_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_wedge_t burton_entity_types.h
   \brief The burton_wedge_t type provides an interface for managing and
          geometry and state associated with mesh wedges.
   \tparam size_t N The domain for the wedge.
 */
template<size_t N>
class burton_wedge_t : public mesh_entity_t<2, N> {
public:

  //! Physics vector type
  using vector_t = burton_mesh_traits_t::vector_t;

  //! Set the corner that a wedge is a part of.
  void set_corner(burton_corner_t<N> *corner) { corner_ = corner; }

  //! Get the corner that a wedge is a part of.
  burton_corner_t<N> *corner() { return corner_; }

  /*!
    \brief Get the side facet normal for the wedege.
    \return vector_t Side facet normal vector.
   */
  vector_t side_facet_normal();

  /*!
    \brief Get the cell facet normal for the wedege.
    \return vector_t Cell facet normal vector.
   */
  vector_t cell_facet_normal();

  /*!
    \brief Set the precedence for the wedge.
    \param size_t dim Topological dimension of the wedge.
    \param uint64_t precedence Value of precedence to set.
   */
  void set_precedence(size_t dim, uint64_t precedence) {}

  /*!
    \brief create_entities function for burton_wedge_t.
   */
  std::pair<size_t, std::vector<size_t>> create_entities(
    size_t dim, std::vector<flexi::id_t> &e, id_t *v, size_t vertex_count) {
    e.resize(6);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[0];
    e[5] = v[2];

    return {3, {2, 2, 2}};
  }

private:

  burton_corner_t<N> *corner_;

}; // struct burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_corner_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_corner_t burton_entity_types.h
   \brief The burton_corner_t type provides an interface for managing and
          geometry and state associated with mesh corners.
   \tparam size_t N The domain for the corner.
 */
template<size_t N>
class burton_corner_t : public mesh_entity_t<1, N> {
public:
  /*!
    \brief Add a wedge to the mesh.
    \param burton_wedge_t<N> *w The wedge to add to the mesh.
   */
  void add_wedge(burton_wedge_t<N> *w) {
    wedges_.add(w);
    w->set_corner(this);
  }

  /*!
    \brief Get the wedges for the mesh.
    \return entity_group<burton_wedge_t<N>> &wedges The wedges in the mesh.
   */
  entity_group<burton_wedge_t<N>> &wedges() { return wedges_; } // wedges

private:
  entity_group<burton_wedge_t<N>> wedges_;

}; // class burton_corner_t

} // namespace flexi

#endif // flexi_burton_entity_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
