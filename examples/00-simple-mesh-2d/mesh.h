
#pragma once 

#include <array>
#include <iostream>
#include <vector>

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/mesh.h>
#include <flecsi/topology/mesh_topology.h>

namespace flecsi{
namespace simple_mesh{

/******************************************************************************/
// Define a mesh type: This involves first defining the entity types that the
// mesh consists of, the mesh policy that specifies all the entity types as well
// as the adjacency/binding maps that the mesh should compute. Finally, the mesh
// type is defined using the specified mesh policy to define the full type of 
// the specialized mesh.  
/******************************************************************************/

//----------------------------------------------------------------------------//
// Point type.
//----------------------------------------------------------------------------//

using point_t = std::array<double, 2>;

//----------------------------------------------------------------------------//
// Vertex type.
//----------------------------------------------------------------------------//

struct vertex_t : public flecsi::topology::mesh_entity_u<0, 1> {

  vertex_t(point_t & p) : p_(p) {}

  point_t const & coordinates() const {
    return p_;
  }

  void print(const char * string) {
    std::cout << string << " My id is " << id<0>() << std::endl;
  } // print

private:
  point_t p_;

}; // struct vertex_t

//----------------------------------------------------------------------------//
// Edge type.
//----------------------------------------------------------------------------//

struct edge_t : public flecsi::topology::mesh_entity_u<1, 1> {
}; // struct edge_t

//----------------------------------------------------------------------------//
// Cell type.
//----------------------------------------------------------------------------//

struct cell_t : public flecsi::topology::mesh_entity_u<2, 1> {
  using id_t = flecsi::utils::id_t;

  void print(const char *string) {
    std::cout << string << " My id is " << id<0>() << std::endl;
  } // print

  std::vector<size_t> create_entities(id_t cell_id,
    size_t dim,
    flecsi::topology::domain_connectivity_u<2> & c,
    id_t * e) {

    id_t * v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[2];

    e[2] = v[1];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[1];

    e[6] = v[2];
    e[7] = v[3];

    return {2, 2, 2, 2};
  } // create_entities

}; // struct cell_t

//----------------------------------------------------------------------------//
// Define an enum type "index_spaces" to assign an id to distinguish between 
// the index-spaces for entity types as well as the adjacency/connectivity maps.  
//----------------------------------------------------------------------------//

enum index_spaces : size_t {
  vertices,
  edges,
  cells,
  vertices_to_cells,
  cells_to_vertices
}; // enum index_spaces


//----------------------------------------------------------------------------//
// Define a mesh policy by listing all the entity types and various adjacency 
// or connectivity that the mesh type should contain                          
//----------------------------------------------------------------------------//

struct simple_mesh_2d_policy_t {

  using id_t = flecsi::utils::id_t;

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(index_spaces::vertices, 0, vertex_t),
    flecsi_entity_type(index_spaces::cells, 0, cell_t));

  flecsi_register_connectivities(
    flecsi_connectivity(index_spaces::cells_to_vertices, 0, cell_t, vertex_t));

  flecsi_register_bindings();

  template<size_t M, size_t D, typename ST>
  static flecsi::topology::mesh_entity_base_u<1> * create_entity(
    flecsi::topology::mesh_topology_base_u<ST> * mesh,
    size_t num_vertices,
    id_t const & id) {
    return nullptr;
  } // create_entity

}; // struct simple_mesh_2d_policy_t

//----------------------------------------------------------------------------//
// Define a simple 2d mesh using the policy type defined earlier      
//----------------------------------------------------------------------------//

struct simple_mesh_2d_t
  : public flecsi::topology::mesh_topology_u<simple_mesh_2d_policy_t> {

  void print(const char * string) {
    std::cout << string << std::endl;
  } // print

  auto cells() {
    return entities<2, 0>();
  } // cells

//  auto cells(partition_t p) {
//    return entities<2, 0>(p);
//  } // cells

  auto vertices() {
    return entities<0, 0>();
  } // vertices

  template<typename E, size_t M>
  auto vertices(flecsi::topology::domain_entity_u<M, E> & e) {
    return entities<0, 0>(e);
  } // vertices

  template<typename E, size_t M>
  auto centroid(flecsi::topology::domain_entity_u<M, E> & e) {
  
   point_t cen{0.0, 0.0}; 
   for (auto v: vertices(e)){
    point_t coords = v->coordinates();
    cen[0] += coords[0]; 
    cen[1] += coords[1]; 
   }

   cen[0] /= 4; 
   cen[1] /= 4;

   return cen; 
  }

}; // simple_mesh_2d_t

//----------------------------------------------------------------------------//
// Type aliases  
//----------------------------------------------------------------------------//
using mesh_t = simple_mesh_2d_t;

template<size_t PRIVILEGES>
using mesh_handle_t = data_client_handle_u<mesh_t, PRIVILEGES>;


/******************************************************************************/
/* Define an initialization mesh task that will be invoked at the specialization
 * spmd init control point to actually create the entities of the mesh.        */ 
/******************************************************************************/
void
initialize_mesh(mesh_handle_t<wo> m) {
  auto & context = execution::context_t::instance();

  auto & vertex_map{context.index_map(index_spaces::vertices)};
  auto & reverse_vertex_map{context.reverse_index_map(index_spaces::vertices)};
  auto & cell_map{context.index_map(index_spaces::cells)};

  std::vector<vertex_t *> vertices;

  const size_t width{16};

  // make vertices
  for(auto & vm : vertex_map) {
    const size_t mid{vm.second};
    const size_t row{mid / (width + 1)};
    const size_t column{mid % (width + 1)};
     printf("vertex %lu: (%lu, %lu)\n", mid, row, column);
    point_t point({{(double)row, (double)column}});

    vertices.push_back(m.make<vertex_t>(point));
  } // for

  // make cells
  std::unordered_map<size_t, cell_t *> cells_vs_ids;

  for(auto & cm : cell_map) {
    const size_t mid{cm.second};

    const size_t row{mid / width};
    const size_t column{mid % width};

    const size_t v0{(column) + (row) * (width + 1)};
    const size_t v1{(column + 1) + (row) * (width + 1)};
    const size_t v2{(column + 1) + (row + 1) * (width + 1)};
    const size_t v3{(column) + (row + 1) * (width + 1)};

    const size_t lv0{reverse_vertex_map[v0]};
    const size_t lv1{reverse_vertex_map[v1]};
    const size_t lv2{reverse_vertex_map[v2]};
    const size_t lv3{reverse_vertex_map[v3]};

    auto c{m.make<cell_t>()};
    cells_vs_ids[mid] = c;
    m.init_cell<0>(
      c, {vertices[lv0], vertices[lv1], vertices[lv2], vertices[lv3]});
  } // for

  //This method needs to be called to set up the adjacencies between
  //entities.
  m.init<0>();

} //initialize mesh

} // namespace simple_mesh
} // namespace flecsi
