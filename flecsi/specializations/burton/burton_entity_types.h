/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_burton_entity_types_h
#define flecsi_burton_entity_types_h

#include "flecsi/specializations/burton/burton_mesh_traits.h"
#include "flecsi/mesh/mesh_types.h"

/*!
 * \file burton_entity_types.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi
{
/*----------------------------------------------------------------------------*
 * class burton_vertex_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_vertex_t burton_entity_types.h
  \brief The burton_vertex_t type provides an interface for managing
    geometry and state associated with mesh vertices.

  \tparam N The domain of the vertex.
 */
class burton_vertex_t
  : public mesh_entity_t<0, burton_mesh_traits_t::num_domains>
{
public:

  //! Type containing coordinates of the vertex.
  using point_t = burton_mesh_traits_t::point_t;

  //! Handle for accessing state at vertex.
  using data_t = burton_mesh_traits_t::data_t;

  //! Number of domains in the burton mesh.
  static constexpr size_t num_domains = burton_mesh_traits_t::num_domains;

  //! Constructor
  burton_vertex_t(mesh_topology_base_t & mesh)
    : mesh_(mesh) {}

  /*!
    \brief Set the coordinates for a vertex.

    \param[in] coordinates Coordinates value to set for vertex.
   */
  void set_coordinates(const point_t & coordinates)
  {
    auto c = data_t::instance().dense_accessor<point_t, flecsi_internal>(
      "coordinates", mesh_.runtime_id());
    c[mesh_entity_base_t<num_domains>::template id<0>()] = coordinates;
  } // set_coordinates

  /*!
    \brief Get the coordinates at a vertex from the state handle.
    \return coordinates of vertex.
   */
  const point_t & coordinates() const
  {
    const auto c = data_t::instance().dense_accessor<point_t, flecsi_internal>(
      "coordinates", mesh_.runtime_id());
    return c[mesh_entity_base_t<num_domains>::template id<0>()];
  } // coordinates

  mesh_topology_base_t & mesh_;

}; // class burton_vertex_t

/*----------------------------------------------------------------------------*
 * class burton_edge_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_edge_t burton_entity_types.h
  \brief The burton_edge_t type provides an interface for managing
    geometry and state associated with mesh edges.

  \tparam N The domain of the edge.
 */
struct burton_edge_t
    : public mesh_entity_t<1, burton_mesh_traits_t::num_domains> {

  //! Type of floating point.
  using real_t = burton_mesh_traits_t::real_t;

  //! Type containing coordinates of the vertex.
  using point_t = burton_mesh_traits_t::point_t;

  //! Type vector type.
  using vector_t = burton_mesh_traits_t::vector_t;

  //! the constructor
  burton_edge_t(mesh_topology_base_t & mesh) : mesh_(mesh) {}
  
  //! the edge midpoint
  point_t midpoint() const;

  //! the edge length
  real_t  length() const;

  //! the edge normal
  vector_t normal() const;

  //! a reference to the mesh topology
  mesh_topology_base_t & mesh_;

}; // struct burton_edge_t

class burton_corner_t; // class burton_corner_t

class burton_wedge_t; // class burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_cell_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_cell_t burton_entity_types.h
  \brief The burton_cell_t type provides an interface for managing and
    geometry and state associated with mesh cells.
 */
struct burton_cell_t
    : public mesh_entity_t<2, burton_mesh_traits_t::num_domains> {
  mesh_topology_base_t & mesh_;

  //! Type containing coordinates of the vertex.
  using point_t = burton_mesh_traits_t::point_t;

  //! Type of floating point.
  using real_t = burton_mesh_traits_t::real_t;

  //! Constructor
  burton_cell_t(mesh_topology_base_t & mesh) : mesh_(mesh) {}
  //! Destructor
  virtual ~burton_cell_t() {}

  //! the centroid
  virtual point_t centroid() const = 0;

  //! the area of the cell
  virtual real_t area() const  = 0;

  /*!
    \brief create_entities is a function that creates entities
      of topological dimension dim, using vertices v, and puts the vertices
      in e. See, e.g., burton_quadrilateral_cell_t for an implementation of
      this pure virtual function.

    \param[in] dim The topological dimension of the entity to create.
    \param[out] e Vector to fill with ids of the vertices making the entity.
    \param[in] v Vertex ids for the cell.
    \param[in] vertex_count The number of vertices making up the entity.

    \return A pair with a) the number of vertex collections making up the
      entity and b) the number of vertices per collection.
   */
  virtual
  std::vector<size_t>
  create_entities(
    flecsi::id_t cell_id, 
    size_t dim, 
    domain_connectivity<2> & c, 
    flecsi::id_t * e
  ) = 0;

  /*!
    \brief create_bound_entities binds mesh entities across domains.
      See, e.g., burton_quadrilateral_cell_t for an implementation of
      this pure virtual function.

    \param[in] dim The topological dimension of the entity being bound.
    \param[in] ent_ids The entity ids of the entities making up the binding.
    \param[out] c The collection of the ids making up the bound entity.

    \return A pair with a) the number of entity collections making up the
      binding and b) the number of entities per collection.
   */
  virtual flecsi::index_vector_t create_bound_entities(size_t from_domain,
      size_t to_domain, size_t dim, 
      flecsi::id_t cell_id,
      domain_connectivity<2>& primal_conn,
      domain_connectivity<2>& domain_conn,
      id_t * c) = 0;

}; // class burton_cell_t

/*----------------------------------------------------------------------------*
 * class burton_quadrilateral_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_quadrilateral_t burton_entity_types.h

  \brief The burton_quadrilateral_t type provides a derived instance of
    burton_cell_t for 2D quadrilateral cells.
 */
class burton_quadrilateral_cell_t : public burton_cell_t
{
public:
  burton_quadrilateral_cell_t(mesh_topology_base_t & mesh) : burton_cell_t(mesh)
  {
  }

  //! the centroid
  point_t centroid() const override;

  //! the area of the cell
  real_t area() const override;

  /*!
    \brief create_entities function for burton_quadrilateral_cell_t.
   */
  std::vector<size_t>
  create_entities(
    flecsi::id_t cell_id, 
    size_t dim, 
    domain_connectivity<2> & c, 
    flecsi::id_t * e
  ) override
  {
    flecsi::id_t* v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[2];
    e[5] = v[3];

    e[6] = v[3];
    e[7] = v[0];

    return {2, 2, 2, 2};
  } // create_entities

  /*!
    \brief create_bound_entities function for burton_quadrilateral_cell_t.

    \verbatim

    The following shows the labeling of the primitives making up a cell. Given
    vertices v*, edges e*, and center vertex cv.

    v3------e2-------v2
    |                 |
    |                 |
    |                 |
    |                 |
    e3      cv       e1
    |                 |
    |                 |
    |                 |
    |                 |
    v0------e0-------v1

    A wedge is defined by a vertex, an edge, and the cell itself. The wedge
    indexing is shown below.

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

    A corner is defined by a vertex and two edges.

    c0 = {v0, e0, e3}
    c1 = {v1, e0, e1}
    c2 = {v2, e1, e2}
    c3 = {v3, e2, e3}

    \endverbatim
   */
  inline flecsi::index_vector_t create_bound_entities(size_t from_domain,
      size_t to_domain, size_t dim, 
      flecsi::id_t cell_id,
      domain_connectivity<2>& primal_conn,
      domain_connectivity<2>& domain_conn,
      id_t * c
  ) override
  {
    flecsi::id_t* v = primal_conn.get_entities(cell_id, 0);
    flecsi::id_t* e = primal_conn.get_entities(cell_id, 1);

    switch (dim) {
      // Corners
      case 0:
        // corner 0
        c[0] = v[0]; // vertex 0
        c[1] = e[0]; // edge 0, abuts vertex 0
        c[2] = e[3]; // edge 3, abuts vertex 0

        // corner 1
        c[3] = v[1]; // vertex 1
        c[4] = e[0]; // edge 0, abuts vertex 1
        c[5] = e[1]; // edge 1, abuts vertex 1

        // corner 2
        c[6] = v[2]; // vertex 2
        c[7] = e[1]; // edge 1, abuts vertex 2
        c[8] = e[2]; // edge 2, abuts vertex 2

        // corner 3
        c[9] = v[3]; // vertex 3
        c[10] = e[2]; // edge 2, abuts vertex 3
        c[11] = e[3]; // edge 3, abuts vertex 3

        return {3, 3, 3, 3};

      // Wedges
      case 1:

        // wedge 0
        c[0] = v[0]; // vertex 0
        c[1] = e[3]; // edge 3

        // wedge 1
        c[2] = v[0]; // vertex 0
        c[3] = e[0]; // edge 0

        // wedge 2
        c[4] = v[1]; // vertex 1
        c[5] = e[0]; // edge 0

        // wedge 3
        c[6] = v[1]; // vertex 1
        c[7] = e[1]; // edge 1

        // wedge 4
        c[8] = v[2]; // vertex 2
        c[9] = e[1]; // edge 1

        // wedge 5
        c[10] = v[2]; // vertex 2
        c[11] = e[2]; // edge 2

        // wedge 6
        c[12] = v[3]; // vertex 3
        c[13] = e[2]; // edge 2

        // wedge 7
        c[14] = v[3]; // vertex 3
        c[15] = e[3]; // edge 3

        return {2, 2, 2, 2, 2, 2, 2, 2};

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

  \tparam N The domain of the wedge.
 */
class burton_wedge_t
    : public mesh_entity_t<1, burton_mesh_traits_t::num_domains>
{
public:
  burton_wedge_t(){}

  burton_wedge_t(mesh_topology_base_t & mesh){}

  //! Physics vector type.
  using vector_t = burton_mesh_traits_t::vector_t;

  //! Set the cell that a wedge is in.
  void set_cell(burton_cell_t * cell) { cell_ = cell; }

  //! Set the edge that a wedge has.
  void set_edge(burton_edge_t * edge) { edge_ = edge; }

  //! Set the vertex that a wedge has.
  void set_vertex(burton_vertex_t * vertex) { vertex_ = vertex; }

  //! Set the corner that a wedge is in.
  void set_corner(burton_corner_t * corner) { corner_ = corner; }

  //! Get the cell that a wedge is in.
  const burton_cell_t * cell() const { return cell_; }

  //! Get the edge that a wedge has.
  const burton_edge_t * edge() const { return edge_; }

  //! Get the vertex that a wedge has.
  const burton_vertex_t * vertex() const { return vertex_; }

  //! Get the corner that a wedge is in.
  const burton_corner_t * corner() const { return corner_; }

  /*!
    \brief Get the side facet normal for the wedge.
    \return Side facet normal vector.
   */
  vector_t side_facet_normal() const
  {
    // Use the edge midpoint and the cell centroid to create the normal vector.
    // Multiply normal by the sign of the dot between the normal and the vector
    // from v to e to get "outward facing" normal.
    auto c = cell()->centroid();
    auto e = edge()->midpoint();
    auto v = vertex()->coordinates();
    auto nrml = normal(c,e);
    return nrml*sgn(dot(nrml,point_to_vector(e-v)));
  }

  /*!
    \brief Get the cell facet normal for the wedge.
    \return Cell facet normal vector.
   */
  vector_t cell_facet_normal() const
  {
    // Use the edge midpoint and vertex to create the normal vector.
    // Multiply normal by the sign of the dot between the normal and the vector
    // from c to e to get "outward facing" normal.
    auto e = edge()->midpoint();
    auto v = vertex()->coordinates();
    auto c = cell()->centroid();
    auto nrml = normal(e,v);
    return nrml*sgn(dot(nrml,point_to_vector(e-c)));
  }

private:

  /*!
    \function sgn

    \tparam T type of data to work on

    \returns 1 if positive, -1 if negative.
   */
  template <typename T>
  T sgn(T val) const {
    return (T(0) < val) - (val < T(0));
  }

  burton_cell_t * cell_;
  burton_edge_t * edge_;
  burton_vertex_t * vertex_;
  burton_corner_t * corner_;

}; // struct burton_wedge_t

/*----------------------------------------------------------------------------*
 * class burton_corner_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_corner_t burton_entity_types.h
  \brief The burton_corner_t type provides an interface for managing and
    geometry and state associated with mesh corners.

  \tparam N The domain of the corner.
 */
class burton_corner_t
    : public mesh_entity_t<0, burton_mesh_traits_t::num_domains>
{
public:

  //! Type of floating point.
  using real_t = burton_mesh_traits_t::real_t;

  //! Type containing coordinates of a vertex.
  using point_t = burton_mesh_traits_t::point_t;

  //! Type vector type.
  using vector_t = burton_mesh_traits_t::vector_t;

  mesh_topology_base_t & mesh_;

  burton_corner_t(mesh_topology_base_t & mesh) : mesh_(mesh) {}

  /*!
    \brief Add a wedge to the mesh.

    \param[in] w The wedge to add to the mesh.
   */
  void add_wedge(burton_wedge_t * w)
  {
    wedges_.add(w);
    w->set_corner(this);
  }

  //! Set the cell that a corner is in.
  void set_cell(burton_cell_t * cell) { cell_ = cell; }

  //! Set the first edge that a corner has.
  void set_edge1(burton_edge_t * edge) { edge1_ = edge; }

  //! Set the second edge that a corner has.
  void set_edge2(burton_edge_t * edge) { edge2_ = edge; }

#if 0
  // FIXME: having to set/get the edges 1 and 2 is hacky. Need to review
  // this with Bergen to come up with a cleaner solution.
  using entity_set_t = entity_set_t<1,1>;
  //! Set the edges that a corner has.
  void set_edges(entity_set_t edges) { edges_ = edges; }
#endif

  //! Set the vertex that a corner has.
  void set_vertex(burton_vertex_t * vertex) { vertex_ = vertex; }

  //! Get the cell that a corner is in.
  const burton_cell_t * cell() const { return cell_; }

  //! Get edge1 that a corner has.
  const burton_edge_t * edge1() const { return edge1_; }

  //! Get edge2 that a corner has.
  const burton_edge_t * edge2() const { return edge2_; }

#if 0
  //! Get edges that a corner has.
  const entity_set_t & edges() const { return edges_; }
#endif

  //! Get the vertex that a corner has.
  const burton_vertex_t * vertex() const { return vertex_; }

  //! the area of the corner
  real_t area() {
    /*
      Area of a quadrilateral. No sides parallel.

                 D
          /-------------\
         /                \
      B /                   \  C
       /                      \
      /-------------------------\
                 A

      area = 1/2 mag(A X B) + 1/2 mag(C X D)
     */

    real_t area = 0.0;

    auto xc = cell()->centroid();
    auto xv = vertex()->coordinates();

    vector_t A(point_to_vector(edge1()->midpoint() - xv));
    vector_t B(point_to_vector(edge2()->midpoint() - xv));
    vector_t C(point_to_vector(edge1()->midpoint() - xc));
    vector_t D(point_to_vector(edge2()->midpoint() - xc));

#if 0
    vector_t A(point_to_vector(edges()[0]->midpoint() - xv));
    vector_t B(point_to_vector(edges()[1]->midpoint() - xv));
    vector_t C(point_to_vector(edges()[0]->midpoint() - xc));
    vector_t D(point_to_vector(edges()[1]->midpoint() - xc));
#endif

    return 0.5*(cross_magnitude(A,B) + cross_magnitude(C,D));
  }

  /*!
    \brief Get the wedges for the mesh.
    \return The wedges in the mesh.
   */
  entity_group<burton_wedge_t> & wedges() { return wedges_; } // wedges
private:
  entity_group<burton_wedge_t> wedges_;
  burton_cell_t * cell_;
  burton_edge_t * edge1_;
  burton_edge_t * edge2_;
#if 0
  entity_set_t  edges_;
#endif
  burton_vertex_t * vertex_;

}; // class burton_corner_t

} // namespace flecsi

#endif // flecsi_burton_entity_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
