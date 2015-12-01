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
    \brief Accessor type.
   */
  template<typename T>
  using accessor_t = burton_mesh_traits_t::mesh_state_t::accessor_t<T>;

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

  using point_t = burton_mesh_traits_t::point_t;
  using vector_t = burton_mesh_traits_t::vector_t;

  using vertex_t = burton_mesh_types_t::vertex_t;
  using edge_t = burton_mesh_types_t::edge_t;

  using cell_t = burton_mesh_types_t::cell_t;
  using quadrilateral_cell_t = burton_mesh_types_t::quadrilateral_cell_t;

  using wedge_t = burton_mesh_types_t::wedge_t;
  using corner_t = burton_mesh_types_t::corner_t;

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
  static constexpr auto dimension() {
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
    return mesh_.num_entities(1, 0);
  } // num_corners

  /*!
    Get number of wedges.
   */
  size_t num_wedges() {
    return mesh_.num_entities(1, 2);
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
  auto vertices(wedge_t *w) { return mesh_.vertices<1>(w); }

  /*!
    FIXME
   */
  template <class E> auto vertices(E *e) { return mesh_.vertices<0>(e); }

  template<size_t M, class E>
  auto vertices(domain_entity<M, E>& e) { return mesh_.vertices(e); } 

  /*!
    FIXME
   */
  template <class E> auto edges(E *e) { return mesh_.edges<0>(e); }

  template<size_t M, class E>
  auto edges(domain_entity<M, E>& e) { return mesh_.edges(e); } 

  /*!
    FIXME
   */
  template <class E> auto cells(E *e) { return mesh_.cells<0>(e); }

  template<size_t M, class E>
  auto cells(domain_entity<M, E>& e) { return mesh_.cells(e); } 

  /*!
    FIXME
   */
  template <class E> auto vertex_ids(E *e) { return mesh_.vertex_ids<0>(e); }

  /*!
    FIXME
   */
  template <class E> auto edge_ids(E *e) { return mesh_.edge_ids<0>(e); }

  /*!
    FIXME
   */
  template <class E> auto cell_ids(E *e) { return mesh_.cell_ids<0>(e); }

  /*!
    Create a vertex in the mesh.

    \param pos The position (coordinates) for the vertex.
   */
  vertex_t *create_vertex(const point_t &pos) {
    auto p = access_state_<point_t>("coordinates");
    p[mesh_.num_vertices()] = pos;

    auto v = mesh_.make<vertex_t>(pos, &state_);
    mesh_.add_vertex<0>(v);
    return v;
  }

  /*!
    Create a cell in the mesh.

    \param verts The vertices defining the cell.
   */
  cell_t * create_cell(std::initializer_list<vertex_t *> verts) {
    // FIXME: Add element types
    cell_t * c = mesh_.make<quadrilateral_cell_t>();
    mesh_.add_cell<0>(c);

    mesh_.init_cell<0>(c, verts);
    return c;
  }

  /*!
    FIXME
   */

  void dump(){
    mesh_.dump();
  }

  /*!
    FIXME
   */
  void init_parameters(size_t vertices) {

    // register coordinate state
    state_.register_state<point_t>("coordinates", vertices,
      attachment_site_t::vertices, persistent);

  } // init_parameters

  /*!
    \verbatim

    After cells and vertices for the mesh have been defined, call init() to
    create the dual mesh entities. The following picture shows the vertex
    definitions used to create the dual mesh. For each vertex definition there
    is a corresponding point_t variable containing the geometry of the vertex,
    which is computed from the primal mesh vertex coordinates.

    v3-----e2-----v2
    |              |
    |              |
    e3     cv     e1
    |              |
    |              |
    v0-----e0-----v1

    The wedge indexing is shown below. A wedge is defined by three vertices in
    the order w = {cv, v*, e*}.

    v3------e2-------v2
    | \      |      / |
    |   \  w6|w5  /   |
    |  w7 \  |  / w4  |
    |       \|/       |
    e3------cv-------e1
    |       /|\       |
    |  w0 /  |  \ w3  |
    |   /  w1|w2  \   |
    | /      |      \ |
    v0------e0-------v1

    Corners are defined by the two wedges sharing the center vertex cv and a
    primal vertex v*.
    c0 = {w0, w1}
    c1 = {w2, w3}
    c2 = {w4, w5}
    c3 = {w6, w7}

    \endverbatim
   */
  void init() {
    mesh_.init<0>();

    // for each cell in the primal mesh
    for (auto c : mesh_.cells<0>()) {
      // vertices for cell c

      auto vs = mesh_.vertices(c).toVec();

      // retrieve points for each of the primal vertices
      auto v0p = vs[0]->coordinates();
      auto v1p = vs[1]->coordinates();
      auto v2p = vs[2]->coordinates();
      auto v3p = vs[3]->coordinates();

      // compute the center vertex
      auto cvp = flexi::centroid( {v0p, v1p, v2p, v3p } );

      // compute the edge points as midpoints of the primal vertices
      // FIXME? flexi:: namespace required so compiler doesn't only look
      // local to this class for midpoint definition. The midpoint defined
      // in this class takes an edge_t.
      auto e0p = flexi::midpoint(v1p, v0p);
      auto e1p = flexi::midpoint(v2p, v1p);
      auto e2p = flexi::midpoint(v3p, v2p);
      auto e3p = flexi::midpoint(v0p, v3p);

      // create the dual mesh vertices using the point_ts
      auto v0 = mesh_.make<vertex_t>(v0p, &state_);
      mesh_.add_vertex<1>(v0);
      v0->set_rank<1>(1);

      auto v1 = mesh_.make<vertex_t>(v1p, &state_);
      mesh_.add_vertex<1>(v1);
      v1->set_rank<1>(1);

      auto v2 = mesh_.make<vertex_t>(v2p, &state_);
      mesh_.add_vertex<1>(v2);
      v2->set_rank<1>(1);

      auto v3 = mesh_.make<vertex_t>(v3p, &state_);
      mesh_.add_vertex<1>(v3);
      v3->set_rank<1>(1);

      auto cv = mesh_.make<vertex_t>(cvp, &state_);
      mesh_.add_vertex<1>(cv);
      cv->set_rank<1>(0);

      auto e0 = mesh_.make<vertex_t>(e0p, &state_);
      mesh_.add_vertex<1>(e0);
      e0->set_rank<1>(1);

      auto e1 = mesh_.make<vertex_t>(e1p, &state_);
      mesh_.add_vertex<1>(e1);
      e1->set_rank<1>(1);

      auto e2 = mesh_.make<vertex_t>(e2p, &state_);
      mesh_.add_vertex<1>(e2);
      e2->set_rank<1>(1);

      auto e3 = mesh_.make<vertex_t>(e3p, &state_);
      mesh_.add_vertex<1>(e3);
      e3->set_rank<1>(1);

      // make wedges using the vertices
      auto w0 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w0);
      mesh_.init_cell<1>(w0, {cv, v0, e3});
      c->add_wedge(w0);

      auto w1 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w1);
      mesh_.init_cell<1>(w1, {cv, v0, e0});
      c->add_wedge(w0);

      auto w2 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w2);
      mesh_.init_cell<1>(w2, {cv, v1, e0});
      c->add_wedge(w2);

      auto w3 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w3);
      mesh_.init_cell<1>(w3, {cv, v1, e1});
      c->add_wedge(w3);

      auto w4 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w4);
      mesh_.init_cell<1>(w4, {cv, v2, e1});
      c->add_wedge(w4);

      auto w5 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w5);
      mesh_.init_cell<1>(w5, {cv, v2, e2});
      c->add_wedge(w5);

      auto w6 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w6);
      mesh_.init_cell<1>(w6, {cv, v3, e2});
      c->add_wedge(w6);

      auto w7 = mesh_.make<wedge_t>();
      mesh_.add_cell<1>(w7);
      mesh_.init_cell<1>(w7, {cv, v3, e3});
      c->add_wedge(w7);

      // make corners using the wedges
      auto c0 = mesh_.make<corner_t>();
      c0->add_wedge(w0);
      c0->add_wedge(w1);
      c->add_corner(c0);

      auto c1 = mesh_.make<corner_t>();
      c1->add_wedge(w2);
      c1->add_wedge(w3);
      c->add_corner(c1);

      auto c2 = mesh_.make<corner_t>();
      c2->add_wedge(w4);
      c2->add_wedge(w5);
      c->add_corner(c2);

      auto c3 = mesh_.make<corner_t>();
      c3->add_wedge(w6);
      c3->add_wedge(w7);
      c->add_corner(c3);
    }

    mesh_.init<1>();
  }

  /*!
    Get the centroid of a cell.
    
    \param[in] cell The cell to return the centroid for.
    \return a point_t that is the centroid.

    FIXME : need const iterator for entity group!!!
    point_t centroid(const cell_t *c) const {

  */
  point_t centroid(cell_t *c) {
    // the first vertex of every wedge is the centroid
    auto w0 = *c->wedges().begin();
    auto cp = *mesh_.vertices<1>(w0).begin();
    return cp->coordinates();
  }

  /*!
    Compute the midpoint of an edge.

    \param[in] cell The edge to return the midpoint for.
    \return a point_t that is the midpoint.
  */
  point_t midpoint(const edge_t *e) const {
    auto vs = mesh_.vertices<0>(e).toVec();
    return point_t(flexi::midpoint(vs[0]->coordinates(),vs[1]->coordinates()));
  }

private:

  private_mesh_t mesh_;

  burton_mesh_traits_t::mesh_state_t state_;

}; // class burton_mesh_t

using mesh_t = burton_mesh_t;

} // namespace flexi

#endif // flexi_burton_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
