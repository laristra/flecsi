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

  class burton_vertex_t : public MeshEntity<0> {
  public:
    //! Constructor
    burton_vertex_t() : precedence_(0) {}

    //! Constructor
    burton_vertex_t(const point_t &coordinates)
        : precedence_(0), coordinates_(coordinates) {}

    void setRank(uint8_t rank) { setInfo(rank); }

    uint64_t precedence() const { return 1 << (63 - info()); }

    void setCoordinates(const point_t &coordinates) {
      coordinates_ = coordinates;
    }

    const point_t &coordinates() const { return coordinates_; }

  private:

    uint64_t precedence_;
    point_t coordinates_;

  }; // struct burton_vertex_t

  /*--------------------------------------------------------------------------*
   * class burton_edge_t
   *--------------------------------------------------------------------------*/

  /*!
     \class burton_edge_t burton_types.h
     \brief The burton_edge_t type provides an interface for managing and
            geometry and state associated with mesh edges.
   */

  struct burton_edge_t : public MeshEntity<1> {}; // struct burton_edge_t

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

  class burton_cell_t : public MeshEntity<2> {
  public:
    void addCorner(burton_corner_t *c) { corners_.add(c); }

    EntityGroup<burton_corner_t> &corners() { return corners_; } // corners

    void addWedge(burton_wedge_t *w) { wedges_.add(w); }

    EntityGroup<burton_wedge_t> &wedges() { return wedges_; } // wedges

  private:
    EntityGroup<burton_corner_t> corners_;
    EntityGroup<burton_wedge_t> wedges_;

  }; // class burton_cell_t

  /*--------------------------------------------------------------------------*
   * class burton_wedge_t
   *--------------------------------------------------------------------------*/

  /*!
     \class burton_wedge_t burton_types.h
     \brief The burton_wedge_t type provides an interface for managing and
            geometry and state associated with mesh wedges.
   */

  class burton_wedge_t : public MeshEntity<2> {
  public:
    void setCorner(burton_corner_t *corner) { corner_ = corner; }

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

  class burton_corner_t : public MeshEntity<0> {
  public:
    void addWedge(burton_wedge_t *w) {
      wedges_.add(w);
      w->setCorner(this);
    }

    EntityGroup<burton_wedge_t> &wedges() { return wedges_; } // wedges

  private:
    EntityGroup<burton_wedge_t> wedges_;

  }; // class burton_corner_t

  /*--------------------------------------------------------------------------*
   * Specify mesh parameterizations.
   *--------------------------------------------------------------------------*/

  using EntityTypes = 
    std::tuple<burton_vertex_t, burton_edge_t, burton_cell_t>;

  using TraversalPairs = 
    std::tuple<std::pair<burton_vertex_t, burton_edge_t>,
               std::pair<burton_vertex_t, burton_cell_t>,
               std::pair<burton_edge_t, burton_vertex_t>,
               std::pair<burton_edge_t, burton_cell_t>,
               std::pair<burton_cell_t, burton_vertex_t>,
               std::pair<burton_cell_t, burton_edge_t>>;

  /*--------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t numEntitiesPerCell(size_t dim) {
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

  static constexpr size_t verticesPerCell() { return 4; } // verticesPerCell

  /*--------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t numVerticesPerEntity(size_t dim) {
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

  static void createEntities(
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

  using EntityTypes =
      std::tuple<burton_vertex_t, burton_edge_t, burton_wedge_t>;

  using TraversalPairs = 
    std::tuple<std::pair<burton_vertex_t, burton_edge_t>,
               std::pair<burton_vertex_t, burton_wedge_t>,
               std::pair<burton_edge_t, burton_vertex_t>,
               std::pair<burton_edge_t, burton_wedge_t>,
               std::pair<burton_wedge_t, burton_vertex_t>,
               std::pair<burton_wedge_t, burton_edge_t>>;

  static size_t numEntitiesPerCell(size_t dim) {
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

  static constexpr size_t verticesPerCell() { return 3; }

  static size_t numVerticesPerEntity(size_t dim) {
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

  static void createEntities(
    size_t dim, std::vector<flexi::id_t> &e, id_t *v) {
    assert(dim = 1);
    assert(e.size() == 6);

    e[0] = v[0];
    e[1] = v[2];

    e[2] = v[1];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[1];
  }
}; // burton_dual_mesh_t

} // namespace flexi

#endif // flexi_burton_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
