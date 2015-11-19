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

class burton_vertex_t : public mesh_entity<0>
{
public:

  using point_t = burton_mesh_traits_t::point_t;
  using state_t = burton_mesh_traits_t::mesh_state_t;

  //! Constructor
  burton_vertex_t() : precedence_(0) {}

  //! Constructor
  burton_vertex_t(const point_t & coordinates, state_t * state)
      : precedence_(0), coordinates_(coordinates), state_(state) {}

  void set_rank(uint8_t rank) { set_info(rank); }

  uint64_t precedence() const { return 1 << (63 - info()); }

  void set_coordinates(const point_t &coordinates) {
    auto c = state_->accessor<point_t>("coordinates");
    coordinates_ = coordinates;
  }

  const point_t &coordinates() const { return coordinates_; }

private:

  uint64_t precedence_;
  point_t coordinates_;
  state_t * state_;

}; // class burton_vertex_t

/*----------------------------------------------------------------------------*
 * class burton_edge_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_edge_t burton_types.h
   \brief The burton_edge_t type provides an interface for managing and
          geometry and state associated with mesh edges.
 */
struct burton_edge_t : public mesh_entity<1> {}; // struct burton_edge_t

class burton_corner_t;
class burton_wedge_t;

/*----------------------------------------------------------------------------*
 * class burton_cell_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_cell_t burton_types.h
   \brief The burton_cell_t type provides an interface for managing and
          geometry and state associated with mesh cells.
 */
class burton_cell_t : public mesh_entity<2>
{
public:

  // FIXME: How should we initialize entity groups?
  burton_cell_t() {}

  virtual ~burton_cell_t() {}

  void add_corner(burton_corner_t *c) { corners_.add(c); }

  entity_group<burton_corner_t> &corners() { return corners_; } // corners

  void add_wedge(burton_wedge_t *w) { wedges_.add(w); }

  entity_group<burton_wedge_t> &wedges() { return wedges_; } // wedges

  void set_precedence(size_t dim, uint64_t precedence) {}

#define INHERIT 0
#if INHERIT
  virtual std::pair<size_t, size_t> create_entities(size_t dim,
    std::vector<id_t> &e, id_t * v, size_t vertex_count) = 0;
#else
  std::pair<size_t, size_t> create_entities(size_t dim,
    std::vector<id_t> &e, id_t * v, size_t vertex_count) {
    
    e.resize(8);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[2];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[3];

    e[6] = v[1];
    e[7] = v[2];

    return {4, 2};
  } // createEntities
#endif

protected:

  entity_group<burton_corner_t> corners_;
  entity_group<burton_wedge_t> wedges_;

}; // class burton_cell_t

/*----------------------------------------------------------------------------*
 * class burton_quadrilateral_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_quadrilateral_t burton_types.h
  \brief The burton_quadrilateral_t type provides an interface
    for managing and geometry and state associated with mesh cells.
 */
class burton_quadrilateral_cell_t : public burton_cell_t
{
public:

  std::pair<size_t, size_t> create_entities(size_t dim,
    std::vector<id_t> &e, id_t * v, size_t vertex_count) {
    
    e.resize(8);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[2];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[3];

    e[6] = v[1];
    e[7] = v[2];

    return {4, 2};
  } // createEntities

}; // class burton_quadrilateral_cell_t

/*----------------------------------------------------------------------------*
 * class burton_wedge_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_wedge_t burton_types.h
   \brief The burton_wedge_t type provides an interface for managing and
          geometry and state associated with mesh wedges.
 */
class burton_wedge_t : public mesh_entity<2> {
public:

  using vector_t = burton_mesh_traits_t::vector_t;

  void set_corner(burton_corner_t *corner) { corner_ = corner; }

  burton_corner_t *corner() { return corner_; }

  vector_t side_facet_normal();
  vector_t cell_facet_normal();

  void set_precedence(size_t dim, uint64_t precedence) {}

  static std::pair<size_t, size_t> create_entities(
    size_t dim, std::vector<flexi::id_t> &e, id_t *v, size_t vertex_count) {
    e.resize(6);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[0];
    e[5] = v[2];

    return {3, 2};
  }

private:

  burton_corner_t *corner_;

}; // struct burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_corner_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_corner_t burton_types.h
   \brief The burton_corner_t type provides an interface for managing and
          geometry and state associated with mesh corners.
 */

class burton_corner_t : public mesh_entity<0> {
public:
  void add_wedge(burton_wedge_t *w) {
    wedges_.add(w);
    w->set_corner(this);
  }

  entity_group<burton_wedge_t> &wedges() { return wedges_; } // wedges

private:
  entity_group<burton_wedge_t> wedges_;

}; // class burton_corner_t

} // namespace flexi

#endif // flexi_burton_entity_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
