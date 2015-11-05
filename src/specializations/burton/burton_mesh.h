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

#ifndef flexi_burton_mesh_h
#define flexi_burton_mesh_h

#include <string>

#include "../../state/state.h"
#include "../../execution/task.h"
#include "burton_types.h"

/*!
 * \file burton_mesh.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

/*----------------------------------------------------------------------------*
 * class burton_mesh_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_mesh_t burton.h
  \brief burton_mesh_t A specialization of the flexi low-level mesh topology,
    state and execution models.
 */
class burton_mesh_t
{
private:

  using private_mesh_t = mesh_topology<burton_mesh_types_t>;
  using private_dual_mesh_t = mesh_topology<burton_dual_mesh_types_t>;

public:

#ifndef MESH_EXECUTION_POLICY
  // for now: use default execution policy
  using mesh_execution_t = execution_t<>;
#else
  using mesh_execution_t = execution_t<MESH_STORAGE_POLICY>;
#endif

  /*!
    \brief Type defining the data attachment sites on the mesh.
   */
  using attachment_site_t = burton_mesh_traits_t::attachment_site_t;

  /*!
    \brief Register state for the named variable at the given attachment
    site.

    \param key A const_string_t name for the state variable, e.g., "density".
    \param site The data attachement site where the state variable should
      be defined.  Valid sites are defined in flexi::burton_mesh_traits_t.
    \param attributes A bitfield specifying various attributes of the state.

    \return An accessor to the newly registered state.
   */
  template<typename T>
  decltype(auto) register_state_(const const_string_t && key,
    attachment_site_t site, bitfield_t::field_type_t attributes = 0x0) {

    switch(site) {
      case attachment_site_t::vertices:
        return state_.register_state<T>(key, num_vertices(),
          attachment_site_t::vertices, attributes);
        break;
      case attachment_site_t::edges:
        return state_.register_state<T>(key, num_edges(),
          attachment_site_t::edges, attributes);
        break;
      case attachment_site_t::cells:
        return state_.register_state<T>(key, num_cells(),
          attachment_site_t::cells, attributes);
        break;
      case attachment_site_t::corners:
        return state_.register_state<T>(key, num_corners(),
          attachment_site_t::corners, attributes);
        break;
      case attachment_site_t::wedges:
        return state_.register_state<T>(key, num_wedges(),
          attachment_site_t::wedges, attributes);
        break;
      default:
        assert(false && "Error: invalid state registration site.");
    } // switch

  } // register_state_

  /*!
    FIXME
   */
  template<typename T>
  decltype(auto) access_state_(const const_string_t && key) {
    return state_.accessor<T>(key);
  } // access_state_

  /*!
    FIXME
   */
  template<typename T>
  decltype(auto) access_type_() {
    return state_.accessors<T>();
  } // access_type_

  /*!
    FIXME
   */
  template<typename T, typename P>
  decltype(auto) access_type_if_(P && predicate) {
    return state_.accessors<T,P>(std::forward<P>(predicate));
  } // access_type_if

  /*!
    FIXME
   */
  decltype(auto) state_attributes_(const const_string_t && key) {
    return state_.meta_data<>((key)).attributes;
  } // state_attribtutes_

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

  /*!
    FIXME
   */
  decltype(auto) dimension() const {
    return burton_mesh_traits_t::dimension;
  } // dimension

  /*!
    Get number of mesh vertices.
   */
  size_t num_vertices() const {
    return mesh_.num_vertices<0>();
  } // num_vertices

  /*!
    Get number of mesh edges.
   */
  size_t num_edges() const { return mesh_.num_edges<0>(); } // num_edges

  /*!
    Get number of mesh cells.
   */
  size_t num_cells() const { return mesh_.num_cells<0>(); } // num_cells

  /*!
    Get number of corners.
   */
  size_t num_corners() {
    return dual_mesh_.num_entities(0, 0);
  } // num_corners

  /*!
    Get number of wedges.
   */
  size_t num_wedges() {
    return dual_mesh_.num_entities(0, 2);
  } // num_wedges

  /*!
    FIXME
   */
  auto vertices() { return mesh_.vertices<0>(); }

  /*!
    FIXME
   */
  auto edges() { return mesh_.edges<0>(); }

  /*!
    FIXME
   */
  auto cells() { return mesh_.cells<0>(); }

  /*!
    FIXME
   */
  auto vertex_ids() { return mesh_.vertex_ids<0>(); }

  /*!
    FIXME
   */
  auto edge_ids() { return mesh_.edge_ids<0>(); }

  /*!
    FIXME
   */
  auto cell_ids() { return mesh_.cell_ids<0>(); }

  /*!
    FIXME
   */
  auto vertices(wedge_t *w) { return dual_mesh_.vertices(w); }

  /*!
    FIXME
   */
  template <class E> auto vertices(E *e) { return mesh_.vertices(e); }

  /*!
    FIXME
   */
  template <class E> auto edges(E *e) { return mesh_.edges(e); }

  /*!
    FIXME
   */
  template <class E> auto cells(E *e) { return mesh_.cells(e); }

  /*!
    FIXME
   */
  template <class E> auto vertex_ids(E *e) { return mesh_.vertex_ids(e); }

  /*!
    FIXME
   */
  template <class E> auto edge_ids(E *e) { return mesh_.edge_ids(e); }

  /*!
    FIXME
   */
  template <class E> auto cell_ids(E *e) { return mesh_.cell_ids(e); }

  /*!
    Create a vertex in the mesh.

    \param pos The position (coordinates) for the vertex.
   */
  vertex_t *create_vertex(const point_t &pos) {
    auto p = access_state_<point_t>("coordinates");
    p[mesh_.num_vertices()] = pos;

    auto v = mesh_.make<vertex_t>(pos, &state_);
    mesh_.add_vertex(v);
    return v;
  }

  /*!
    Create a cell in the mesh.

    \param verts The vertices defining the cell.
   */
  cell_t *create_cell(std::initializer_list<vertex_t *> verts) {
    assert(verts.size() == burton_mesh_types_t::vertices_per_cell() &&
        "vertices size mismatch");
    auto c = mesh_.make<cell_t>();
    mesh_.init_cell(c, verts);
    return c;
  }

  /*!
    FIXME
   */

  void dump(){
    mesh_.dump();
    ndump("_________________________________");
    dual_mesh_.dump();
  }

  /*!
    FIXME
   */
  void init_parameters(size_t vertices) {

    // register coordinate state
    state_.register_state<point_t>("coordinates", vertices,
      attachment_site_t::vertices, 0x0);

  } // init_parameters

  /*!
    FIXME
   */
  void init() {
    mesh_.init();

    for (auto c : mesh_.cells()) {
      auto vs = mesh_.vertices(c).toVec();

      point_t cp;
      cp[0] = vs[0]->coordinates()[0] +
          0.5 * (vs[1]->coordinates()[0] - vs[0]->coordinates()[0]);
      cp[1] = vs[0]->coordinates()[1] +
          0.5 * (vs[3]->coordinates()[1] - vs[0]->coordinates()[1]);

      auto cv = dual_mesh_.make<vertex_t>(cp, &state_);
      cv->set_rank(0);

      auto v0 = dual_mesh_.make<vertex_t>(vs[0]->coordinates(), &state_);
      v0->set_rank(1);

      auto v1 = dual_mesh_.make<vertex_t>(vs[3]->coordinates(), &state_);
      v1->set_rank(1);

      auto v2 = dual_mesh_.make<vertex_t>(vs[1]->coordinates(), &state_);
      v2->set_rank(1);

      auto v3 = dual_mesh_.make<vertex_t>(vs[2]->coordinates(), &state_);
      v3->set_rank(1);

      auto w1 = dual_mesh_.make<wedge_t>();
      dual_mesh_.init_cell(w1, {v0, v1, cv});
      c->add_wedge(w1);

      auto w2 = dual_mesh_.make<wedge_t>();
      dual_mesh_.init_cell(w2, {cv, v1, v3});
      c->add_wedge(w2);

      auto w3 = dual_mesh_.make<wedge_t>();
      dual_mesh_.init_cell(w3, {v2, cv, v3});
      c->add_wedge(w3);

      auto w4 = dual_mesh_.make<wedge_t>();
      dual_mesh_.init_cell(w4, {v0, cv, v2});
      c->add_wedge(w4);

      auto c1 = dual_mesh_.make<corner_t>();
      c1->add_wedge(w1);
      c1->add_wedge(w4);
      c->add_corner(c1);

      auto c2 = dual_mesh_.make<corner_t>();
      c2->add_wedge(w1);
      c2->add_wedge(w2);
      c->add_corner(c2);

      auto c3 = dual_mesh_.make<corner_t>();
      c3->add_wedge(w2);
      c3->add_wedge(w3);
      c->add_corner(c3);

      auto c4 = dual_mesh_.make<corner_t>();
      c4->add_wedge(w3);
      c4->add_wedge(w4);
      c->add_corner(c4);
    }

    dual_mesh_.init();
  }

  /*!
    Compute the centroid of a cell.
    
    \param[in] cell The cell entity whose vertex i want.
    \return a point_t that is the centroid  
  */
  decltype(auto) centroid(const cell_t *c) const {
    point_t tmp(0.0);
    auto vert_list = mesh_.vertices(c);
    for ( auto v : vert_list ) 
      tmp += v->coordinates();
    tmp /= vert_list.size();
    return tmp;
  }


private:

  private_mesh_t mesh_;
  private_dual_mesh_t dual_mesh_;

  burton_mesh_traits_t::mesh_state_t state_;

}; // class burton_mesh_t

using mesh_t = burton_mesh_t;

} // namespace flexi

#endif // flexi_burton_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
