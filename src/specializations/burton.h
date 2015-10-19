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

#ifndef flexi_burton_h
#define flexi_burton_h

#include "burton_types.h"

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

/*----------------------------------------------------------------------------*
 * class burton_mesh_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_mesh_t burton.h
  \brief burton_mesh_t provides...
 */

class burton_mesh_t {
private:
  using private_mesh_t = MeshTopology<burton_mesh_types_t>;
  using private_dual_mesh_t = MeshTopology<burton_dual_mesh_types_t>;

public:
  using real_t = burton_mesh_traits_t::real_t;

  using point_t = point<real_t, burton_mesh_traits_t::dimension>;

  using vertex_t = burton_mesh_types_t::burton_vertex_t;
  using edge_t = burton_mesh_types_t::burton_edge_t;
  using cell_t = burton_mesh_types_t::burton_cell_t;
  using wedge_t = burton_mesh_types_t::burton_wedge_t;
  using corner_t = burton_mesh_types_t::burton_corner_t;

  // Iterator types
  using vertex_iterator_t = private_mesh_t::VertexIterator;
  using edge_iterator_t = private_mesh_t::EdgeIterator;
  using cell_iterator_t = private_mesh_t::CellIterator;

  using wedges_at_corner_iterator_t = private_dual_mesh_t::CellIterator;
  using wedges_at_face_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_vertex_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_cell_iterator_t = private_mesh_t::CellIterator;

  //! Default constructor
  burton_mesh_t() {}

  //! Copy constructor (disabled)
  burton_mesh_t(const burton_mesh_t &) = delete;

  //! Assignment operator (disabled)
  burton_mesh_t &operator=(const burton_mesh_t &) = delete;

  //! Destructor
  ~burton_mesh_t() {}

  decltype(auto) dimension() const {
    return burton_mesh_traits_t::dimension;
  } // dimension

  decltype(auto) num_vertices() const {
    return mesh_.numVertices();
  } // numVertices

  decltype(auto) num_cells() const { return mesh_.numCells(); } // numCells

  /*!
   */
  auto vertices() { return mesh_.vertices(); }

  auto vertices() const { return mesh_.vertices(); }

  auto edges() { return mesh_.edges(); }

  auto cells() { return mesh_.cells(); }

  auto cells() const { return mesh_.cells(); }

  template <class E> decltype(auto) vertices(const E *e) const {
    return mesh_.vertices(e);
  }

  template <class E> auto vertices(E *e) { return mesh_.vertices(e); }

  auto vertices(wedge_t *w) { return dual_mesh_.vertices(w); }

  template <class E> auto edges(E *e) { return mesh_.edges(e); }

  template <class E> auto cells(E *e) { return mesh_.cells(e); }

  vertex_t *create_vertex(const point_t &pos) {
    auto v = mesh_.make<vertex_t>(pos);
    mesh_.addVertex(v);
    return v;
  }

  cell_t *create_cell(std::initializer_list<vertex_t *> verts) {
    assert(verts.size() == burton_mesh_types_t::verticesPerCell() &&
        "vertices size mismatch");
    auto c = mesh_.make<cell_t>();
    mesh_.addCell(c, verts);
    return c;
  }

  void init() {
    for (auto c : mesh_.cells()) {
      auto vs = mesh_.vertices(c).toVec();

      dual_mesh_.addVertex(vs[0]);
      dual_mesh_.addVertex(vs[1]);
      dual_mesh_.addVertex(vs[2]);
      dual_mesh_.addVertex(vs[3]);

      point_t cp;
      cp[0] = vs[0]->coordinates()[0] +
          0.5 * (vs[2]->coordinates()[0] - vs[0]->coordinates()[0]);
      cp[1] = vs[0]->coordinates()[1] +
          0.5 * (vs[1]->coordinates()[1] - vs[0]->coordinates()[1]);

      auto cv = dual_mesh_.make<vertex_t>(cp);
      dual_mesh_.addVertex(cv);
      cv->setRank(0);

      auto w1 = dual_mesh_.make<wedge_t>();
      dual_mesh_.addCell(w1, {vs[0], vs[1], cv});
      c->addWedge(w1);

      auto w2 = dual_mesh_.make<wedge_t>();
      dual_mesh_.addCell(w2, {cv, vs[1], vs[3]});
      c->addWedge(w2);

      auto w3 = dual_mesh_.make<wedge_t>();
      dual_mesh_.addCell(w3, {vs[2], cv, vs[3]});
      c->addWedge(w3);

      auto w4 = dual_mesh_.make<wedge_t>();
      dual_mesh_.addCell(w4, {vs[0], cv, vs[2]});
      c->addWedge(w4);

      auto c1 = dual_mesh_.make<corner_t>();
      c1->addWedge(w1);
      c1->addWedge(w4);
      c->addCorner(c1);

      auto c2 = dual_mesh_.make<corner_t>();
      c2->addWedge(w1);
      c2->addWedge(w2);
      c->addCorner(c2);

      auto c3 = dual_mesh_.make<corner_t>();
      c3->addWedge(w2);
      c3->addWedge(w3);
      c->addCorner(c3);

      auto c4 = dual_mesh_.make<corner_t>();
      c4->addWedge(w3);
      c4->addWedge(w4);
      c->addCorner(c4);
    }
  }

private:
  private_mesh_t mesh_;
  private_dual_mesh_t dual_mesh_;

}; // class burton_mesh_t

using mesh_t = burton_mesh_t;

} // namespace flexi

#endif // flexi_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
