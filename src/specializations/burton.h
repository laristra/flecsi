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

#include "../state/state.h"
#include "../execution/task.h"
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

class burton_mesh_t
{
private:
  using private_mesh_t = MeshTopology<burton_mesh_types_t>;
  using private_dual_mesh_t = MeshTopology<burton_dual_mesh_types_t>;

#ifndef MESH_STORAGE_POLICY
  // for now: use default storage policy
  using private_mesh_state_t = state_t<>;
  using private_dual_mesh_state_t = state_t<>;
#else
  using private_mesh_state_t = state_t<MESH_STORAGE_POLICY>;
  using private_dual_mesh_state_t = state_t<MESH_STORAGE_POLICY>;
#endif

#ifndef MESH_EXECUTION_POLICY
  // for now: use default execution policy
  using private_mesh_execution_t = execution_t<>;
#else
  using private_mesh_execution_t = execution_t<MESH_STORAGE_POLICY>;
#endif

public:

  /*--------------------------------------------------------------------------*
   * Execution Interface
   *--------------------------------------------------------------------------*/

#define execute(task, ...) \
  private_mesh_execution_t::execute_task(task, ##__VA_ARGS__)

  /*--------------------------------------------------------------------------*
   * State Interface
   *--------------------------------------------------------------------------*/

  enum class attachment_site_t : size_t {
    vertices,
    edges,
    cells,
    corners,
    wedges
  }; // enum class attachment_sites_t

#define register_state(mesh, key, site, type) \
  (mesh).register_state_<type>((key), \
  burton_mesh_t::attachment_site_t::site)

  template<typename T>
  decltype(auto) register_state_(const const_string_t && key,
    attachment_site_t site) {
    switch(site) {
    case attachment_site_t::vertices:
      return mesh_state_.register_state<T,0>(key, num_vertices());
      break;
#if 0
    case attachment_site_t::edges:
      return mesh_state_.register_state<T,1>(key, num_edges());
      break;
#endif
    case attachment_site_t::cells:
      return mesh_state_.register_state<T,2>(key, num_cells());
      break;
#if 0
    case attachment_site_t::corners:
      return dual_mesh_state_.register_state<T,1>(key, num_corners());
      break;
    case attachment_site_t::wedges:
      return dual_mesh_state_.register_state<T,2>(key, num_wedges());
      break;
#endif
    default:
      assert(false && "Error: invalid state registration site.");
    } // switch
  } // register_state_

#define access_state(mesh, key, site, type) \
  (mesh).access_state_<type>((key), \
  burton_mesh_t::attachment_site_t::site)

  template<typename T>
  decltype(auto) access_state_(const const_string_t && key,
    attachment_site_t site) {
    switch(site) {
    case attachment_site_t::vertices:
      return mesh_state_.accessor<T,0>(key);
      break;
#if 0
    case attachment_site_t::edges:
      return mesh_state_.accessor<T,1>(key);
      break;
#endif
    case attachment_site_t::cells:
      return mesh_state_.accessor<T,2>(key);
      break;
#if 0
    case attachment_site_t::corners:
      return dual_mesh_state_.accessor<T,1>(key);
      break;
    case attachment_site_t::wedges:
      return dual_mesh_state_.accessor<T,2>(key);
      break;
#endif
    default:
      assert(false && "Error: invalid state registration site.");
    } // switch
  } // access_state_

  /*--------------------------------------------------------------------------*
   * FIXME: Other crap
   *--------------------------------------------------------------------------*/

  using real_t = burton_mesh_traits_t::real_t;

  using point_t = point<real_t, burton_mesh_traits_t::dimension>;
  using vector_t = space_vector<real_t, burton_mesh_traits_t::dimension>;

  using vertex_t = burton_mesh_types_t::burton_vertex_t;
  using edge_t = burton_mesh_types_t::burton_edge_t;
  using cell_t = burton_mesh_types_t::burton_cell_t;
  using wedge_t = burton_mesh_types_t::burton_wedge_t;
  using corner_t = burton_mesh_types_t::burton_corner_t;

  // Iterator types
  /*
  using vertex_iterator_t = private_mesh_t::VertexIterator;
  using edge_iterator_t = private_mesh_t::EdgeIterator;
  using cell_iterator_t = private_mesh_t::CellIterator;

  using wedges_at_corner_iterator_t = private_dual_mesh_t::CellIterator;
  using wedges_at_face_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_vertex_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_cell_iterator_t = private_mesh_t::CellIterator;
  */

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

  /*!
    Get number of mesh vertices.
   */
  size_t num_vertices() const {
    return mesh_.numVertices();
  } // num_vertices

  /*!
    Get number of mesh cells.
   */
  size_t num_cells() const { return mesh_.numCells(); } // num_cells

  /*!
   */
  auto vertices() { return mesh_.vertices(); }

  auto edges() { return mesh_.edges(); }

  auto cells() { return mesh_.cells(); }

  template <class E> auto vertices(E *e) { return mesh_.vertices(e); }

  auto vertices(wedge_t *w) { return dual_mesh_.vertices(w); }

  template <class E> auto edges(E *e) { return mesh_.edges(e); }

  template <class E> auto cells(E *e) { return mesh_.cells(e); }

  /*!
    Create a vertex in the mesh.

    \param pos The position (coordinates) for the vertex.
   */
  vertex_t *create_vertex(const point_t &pos) {
    auto v = mesh_.make<vertex_t>(pos);
    mesh_.addVertex(v);
    return v;
  }

  /*!
    Create a cell in the mesh.

    \param verts The vertices defining the cell.
   */
  cell_t *create_cell(std::initializer_list<vertex_t *> verts) {
    assert(verts.size() == burton_mesh_types_t::verticesPerCell() &&
        "vertices size mismatch");
    auto c = mesh_.make<cell_t>();
    mesh_.initCell(c, verts);
    return c;
  }

  void dump(){
    mesh_.dump();
    ndump("_________________________________");
    dual_mesh_.dump();
  }

  void init() {
    for (auto c : mesh_.cells()) {
      auto vs = mesh_.vertices(c).toVec();

      point_t cp;
      cp[0] = vs[0]->coordinates()[0] +
          0.5 * (vs[1]->coordinates()[0] - vs[0]->coordinates()[0]);
      cp[1] = vs[0]->coordinates()[1] +
          0.5 * (vs[3]->coordinates()[1] - vs[0]->coordinates()[1]);

      auto cv = dual_mesh_.make<vertex_t>(cp);
      cv->setRank(0);

      auto v0 = dual_mesh_.make<vertex_t>(vs[0]->coordinates());
      v0->setRank(1);

      auto v1 = dual_mesh_.make<vertex_t>(vs[3]->coordinates());
      v1->setRank(1);

      auto v2 = dual_mesh_.make<vertex_t>(vs[1]->coordinates());
      v2->setRank(1);

      auto v3 = dual_mesh_.make<vertex_t>(vs[2]->coordinates());
      v3->setRank(1);

      auto w1 = dual_mesh_.make<wedge_t>();
      dual_mesh_.initCell(w1, {v0, v1, cv});
      c->addWedge(w1);

      auto w2 = dual_mesh_.make<wedge_t>();
      dual_mesh_.initCell(w2, {cv, v1, v3});
      c->addWedge(w2);

      auto w3 = dual_mesh_.make<wedge_t>();
      dual_mesh_.initCell(w3, {v2, cv, v3});
      c->addWedge(w3);

      auto w4 = dual_mesh_.make<wedge_t>();
      dual_mesh_.initCell(w4, {v0, cv, v2});
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

  private_mesh_state_t mesh_state_;
  private_dual_mesh_state_t dual_mesh_state_;

}; // class burton_mesh_t

using mesh_t = burton_mesh_t;

} // namespace flexi

#endif // flexi_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
