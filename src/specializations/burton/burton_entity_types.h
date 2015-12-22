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

template<size_t N>
class burton_vertex_t : public mesh_entity_t<0, N>
{
public:

  using point_t = burton_mesh_traits_t::point_t;
  using state_t = burton_mesh_traits_t::mesh_state_t;
  static constexpr size_t num_domains = burton_mesh_traits_t::num_domains;

  //! Constructor
  burton_vertex_t(state_t & state)
      : precedence_(0), state_(state) {}

  template<size_t M>
  void set_rank(uint8_t rank) {
    mesh_entity_t<0, N>::template set_info<M>(rank);
  }

  template<size_t M>
  uint64_t precedence() const {
    return 1 << (63 - mesh_entity_t<0, N>::template info<M>());
  } // precedence

  void set_coordinates(const point_t &coordinates) {
    auto c = state_.accessor<point_t,flexi_internal>("coordinates");
    c[mesh_entity_base_t<num_domains>::template id<0>()] = coordinates;
  } // set_coordinates

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
   \class burton_edge_t burton_types.h
   \brief The burton_edge_t type provides an interface for managing and
          geometry and state associated with mesh edges.
 */
template<size_t N>
struct burton_edge_t : public mesh_entity_t<1, N> {}; // struct burton_edge_t

template<size_t N>
class burton_corner_t;

template<size_t N>
class burton_wedge_t;

/*----------------------------------------------------------------------------*
 * class burton_cell_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_cell_t burton_types.h
   \brief The burton_cell_t type provides an interface for managing and
          geometry and state associated with mesh cells.
 */
template<size_t N>
class burton_cell_t : public mesh_entity_t<2, N>
{
public:

  // FIXME: How should we initialize entity groups?
  burton_cell_t() {}

  virtual ~burton_cell_t() {}

  void add_corner(burton_corner_t<N> *c) { corners_.add(c); }

  entity_group<burton_corner_t<N>> &corners() { return corners_; } // corners

  void add_wedge(burton_wedge_t<N> *w) { wedges_.add(w); }

  entity_group<burton_wedge_t<N>> &wedges() { return wedges_; } // wedges

  void set_precedence(size_t dim, uint64_t precedence) {}

  virtual std::pair<size_t, size_t> create_entities(size_t dim,
    std::vector<id_t> &e, id_t * v, size_t vertex_count) = 0;

  void
  create_bound_entities(size_t from_domain,
                        size_t from_dim,
                        size_t to_domain,
                        size_t to_dim,
                        const std::vector<id_t *> ent_ids,
                        const std::vector<size_t> ent_counts,
                        std::vector<mesh_entity_base_t<N> *> & create_entities){

  }

protected:

  entity_group<burton_corner_t<N>> corners_;
  entity_group<burton_wedge_t<N>> wedges_;

}; // class burton_cell_t

/*----------------------------------------------------------------------------*
 * class burton_quadrilateral_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_quadrilateral_t burton_types.h
  \brief The burton_quadrilateral_t type provides an interface
    for managing and geometry and state associated with mesh cells.
 */
template<size_t N>
class burton_quadrilateral_cell_t : public burton_cell_t<N>
{
public:

  /*!
   */
  std::pair<size_t, size_t> create_entities(size_t dim,
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

    return {4, 2};
  } // createEntities

  void
  create_bound_entities(size_t from_domain,
                        size_t from_dim,
                        size_t to_domain,
                        size_t to_dim,
                        const std::vector<id_t *> ent_ids,
                        const std::vector<size_t> ent_counts,
                        std::vector<mesh_entity_base_t<N> *> & create_entities){

  }

}; // class burton_quadrilateral_cell_t

/*----------------------------------------------------------------------------*
 * class burton_wedge_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_wedge_t burton_types.h
   \brief The burton_wedge_t type provides an interface for managing and
          geometry and state associated with mesh wedges.
 */
template<size_t N>
class burton_wedge_t : public mesh_entity_t<2, N> {
public:

  using vector_t = burton_mesh_traits_t::vector_t;

  void set_corner(burton_corner_t<N> *corner) { corner_ = corner; }

  burton_corner_t<N> *corner() { return corner_; }

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

  burton_corner_t<N> *corner_;

}; // struct burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_corner_t
 *----------------------------------------------------------------------------*/

/*!
   \class burton_corner_t burton_types.h
   \brief The burton_corner_t type provides an interface for managing and
          geometry and state associated with mesh corners.
 */

template<size_t N>
class burton_corner_t : public mesh_entity_t<0, N> {
public:
  void add_wedge(burton_wedge_t<N> *w) {
    wedges_.add(w);
    w->set_corner(this);
  }

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
