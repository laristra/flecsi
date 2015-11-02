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

#ifndef flexi_burton_types_h
#define flexi_burton_types_h

#include "../utils/common.h"
#include "../mesh/mesh_topology.h"
#include "../geometry/point.h"
#include "../geometry/space_vector.h"

/*!
 * \file burton_types.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

struct burton_mesh_traits_t {
  static constexpr size_t dimension = 2;
  using real_t = double;

  enum class attachment_site_t : size_t {
    vertices,
    edges,
    cells,
    corners,
    wedges
  }; // enum class attachment_site_t

  enum class state_attribute_t : bitfield_t::field_type_t {
    persistent = 0
  }; // enum class state_attribute_t

  /*--------------------------------------------------------------------------*
   * State type definitions
   *--------------------------------------------------------------------------*/

  struct private_state_meta_data_t {

    void initialize(attachment_site_t site_,
      bitfield_t::field_type_t attributes_) {
      site = site_;
      attributes = attributes_;
    } // initialize

    attachment_site_t site;
    bitfield_t::field_type_t attributes;

  }; // struct private_state_meta_data_t
  
#ifndef MESH_STORAGE_POLICY
  using private_mesh_state_t = state_t<private_state_meta_data_t>;
#else
  using private_mesh_state_t =
    state_t<private_state_meta_data_t, MESH_STORAGE_POLICY>;
#endif
}; // struct burton_mesh_traits_t

/*----------------------------------------------------------------------------*
 * struct burton_mesh_types_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_mesh_types_t burton_types.h
  \brief The burton_mesh_types_t is a collection of type information needed
    to specialize the flexi low-level mesh infrastructure for
    ALE methods.
*/

struct burton_mesh_types_t {

  static constexpr size_t dimension = burton_mesh_traits_t::dimension;
  using real_t = burton_mesh_traits_t::real_t;

  using point_t = point<real_t, dimension>;
  using vector_t = space_vector<real_t, dimension>;

  /*--------------------------------------------------------------------------*
   * class burton_vertex_t
   *--------------------------------------------------------------------------*/

  class burton_vertex_t : public mesh_entity<0> {
  public:
    using state_t = burton_mesh_traits_t::private_mesh_state_t;

    //! Constructor
    burton_vertex_t() : precedence_(0) {}

    //! Constructor
    burton_vertex_t(const point_t &coordinates, state_t * state)
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

  }; // struct burton_vertex_t

  /*--------------------------------------------------------------------------*
   * class burton_edge_t
   *--------------------------------------------------------------------------*/

  /*!
     \class burton_edge_t burton_types.h
     \brief The burton_edge_t type provides an interface for managing and
            geometry and state associated with mesh edges.
   */

  struct burton_edge_t : public mesh_entity<1> {}; // struct burton_edge_t

  class burton_corner_t;
  class burton_wedge_t;

  /*--------------------------------------------------------------------------*
   * class burton_cell_t
   *--------------------------------------------------------------------------*/

  /*!
     \class burton_cell_t burton_types.h
     \brief The burton_cell_t type provides an interface for managing and
            geometry and state associated with mesh cells.
   */

  class burton_cell_t : public mesh_entity<2> {
  public:
    void add_corner(burton_corner_t *c) { corners_.add(c); }

    entity_group<burton_corner_t> &corners() { return corners_; } // corners

    void add_wedge(burton_wedge_t *w) { wedges_.add(w); }

    entity_group<burton_wedge_t> &wedges() { return wedges_; } // wedges

  private:
    entity_group<burton_corner_t> corners_;
    entity_group<burton_wedge_t> wedges_;

  }; // class burton_cell_t

  /*--------------------------------------------------------------------------*
   * class burton_wedge_t
   *--------------------------------------------------------------------------*/

  /*!
     \class burton_wedge_t burton_types.h
     \brief The burton_wedge_t type provides an interface for managing and
            geometry and state associated with mesh wedges.
   */

  class burton_wedge_t : public mesh_entity<2> {
  public:
    void set_corner(burton_corner_t *corner) { corner_ = corner; }

    burton_corner_t *corner() { return corner_; }

    vector_t side_facet_normal();
    vector_t cell_facet_normal();

  private:
    burton_corner_t *corner_;

  }; // struct burton_wedge_t

  /*--------------------------------------------------------------------------*
   * class burton_corner_t
   *--------------------------------------------------------------------------*/

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

  /*--------------------------------------------------------------------------*
   * Specify mesh parameterizations.
   *--------------------------------------------------------------------------*/

  using entity_types =
    std::tuple< 
      std::tuple<burton_vertex_t, burton_edge_t, burton_cell_t>
    >;

  using traversal_pairs = 
    std::tuple<std::pair<burton_vertex_t, burton_edge_t>,
               std::pair<burton_vertex_t, burton_cell_t>,
               std::pair<burton_edge_t, burton_vertex_t>,
               std::pair<burton_edge_t, burton_cell_t>,
               std::pair<burton_cell_t, burton_vertex_t>,
               std::pair<burton_cell_t, burton_edge_t>>;

  /*--------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t num_entities_per_cell(size_t dim) {
    switch (dim) {
    case 0:
      return 4;
    case 1:
      return 4;
    case 2:
      return 1;
    default:
      assert(false && "invalid dimension");
    } // switch
  }   // numEntitiesPerCell

  /*--------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static constexpr size_t vertices_per_cell() { return 4; } // verticesPerCell

  /*--------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t num_vertices_per_entity(size_t dim) {
    switch (dim) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 4;
    default:
      assert(false && "invalid dimension");
    } // switch
  }   // numVerticesPerEntity

  static void create_entities(
    size_t dim, std::vector<flexi::id_t> &e, id_t *v) {
    assert(dim = 1);
    assert(e.size() == 8);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[2];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[3];

    e[6] = v[1];
    e[7] = v[2];
  } // createEntities

}; // struct burton_mesh_types_t

// FIXME
class burton_dual_mesh_types_t {
public:
  static constexpr size_t dimension = burton_mesh_traits_t::dimension;

  using real_t = burton_mesh_traits_t::real_t;

  using point_t = point<real_t, dimension>;

  using burton_vertex_t = burton_mesh_types_t::burton_vertex_t;

  using burton_edge_t = burton_mesh_types_t::burton_edge_t;

  using burton_wedge_t = burton_mesh_types_t::burton_wedge_t;

  using entity_types =
      std::tuple<
        std::tuple<burton_vertex_t, burton_edge_t, burton_wedge_t>
      >;

  using traversal_pairs = 
    std::tuple<std::pair<burton_vertex_t, burton_edge_t>,
               std::pair<burton_vertex_t, burton_wedge_t>,
               std::pair<burton_edge_t, burton_vertex_t>,
               std::pair<burton_edge_t, burton_wedge_t>,
               std::pair<burton_wedge_t, burton_vertex_t>,
               std::pair<burton_wedge_t, burton_edge_t>>;

  static size_t num_entities_per_cell(size_t dim) {
    switch (dim) {
    case 0:
      return 3;
    case 1:
      return 3;
    case 2:
      return 1;
    default:
      assert(false && "invalid dimension");
    }
  }

  static constexpr size_t vertices_per_cell() { return 3; }

  static size_t num_vertices_per_entity(size_t dim) {
    switch (dim) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    default:
      assert(false && "invalid dimension");
    }
  }

  static void create_entities(
    size_t dim, std::vector<flexi::id_t> &e, id_t *v) {
    assert(dim = 1);
    assert(e.size() == 6);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[0];
    e[5] = v[2];
  }
}; // burton_dual_mesh_t

} // namespace flexi

#endif // flexi_burton_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
