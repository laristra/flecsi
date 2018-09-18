#include <mpi.h>
#include <unistd.h>

#define FLECSI_DEPENDENT_PARTITION_MODEL FLECSI_RUNTIME_MODEL_legion

#include <flecsi/execution/dependent_partition.h>
#include <flecsi/topology/mesh_definition.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/coloring/add_colorings_unified.h>


namespace flecsi {
namespace execution {

void
add_colorings_unified(flecsi::supplemental::coloring_map_t& map)    
{
  printf("start DP\n");
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  
  int num_cells = sd.num_entities(1);
  int num_vertices = sd.num_entities(0);
  
  dependent_partition_t dp;
  
  std::vector<int> entity_vector;
  entity_vector.push_back(sd.dimension());
  entity_vector.push_back(0);
  
  
  space_t cells = dp.load_entity(num_cells, sd.dimension(), map.cells, entity_vector, sd);
  
  map_t cell_to_cell = dp.load_cell_to_entity(cells, cells, sd);
  
  set_t cell_primary = dp.partition_by_color(cells);
  
  set_t cell_closure = dp.partition_by_image(cells, cells, cell_to_cell, cell_primary);
  
  set_t cell_ghost = dp.partition_by_difference(cells, cell_closure, cell_primary);
  
  set_t cell_ghost_closure = dp.partition_by_image(cells, cells, cell_to_cell, cell_ghost);
  
  set_t cell_shared = dp.partition_by_intersection(cells, cell_ghost_closure, cell_primary);
  
  set_t cell_exclusive = dp.partition_by_difference(cells, cell_primary, cell_shared);
  
  space_t vertices = dp.load_entity(num_vertices, 0, map.vertices, entity_vector, sd);
  
  map_t cell_to_vertex = dp.load_cell_to_entity(cells, vertices, sd);
  
  set_t vertex_alias = dp.partition_by_image(cells, vertices, cell_to_vertex, cell_primary);
  
  dp.min_reduction_by_color(vertices, vertex_alias);
  
  set_t vertex_primary = dp.partition_by_color(vertices);
  
  set_t vertex_of_ghost_cell = dp.partition_by_image(cells, vertices, cell_to_vertex, cell_ghost);
  
  set_t vertex_ghost = dp.partition_by_difference(vertices, vertex_of_ghost_cell, vertex_primary);
  
  set_t vertex_of_shared_cell = dp.partition_by_image(cells, vertices, cell_to_vertex, cell_shared);
  
  set_t vertex_shared = dp.partition_by_intersection(vertices, vertex_of_shared_cell, vertex_primary);
  
  set_t vertex_exclusive = dp.partition_by_difference(vertices, vertex_primary, vertex_shared);
  
  /*
  legion_entity edges = dp.load_entity(num_vertices, 2, 3);
  
  legion_adjacency cell_to_edge = dp.load_cell_to_entity(cells, edges);
  
  legion_partition edge_alias = dp.partition_by_image(cells, edges, cell_to_edge, cell_primary);
  
  dp.min_reduction_by_color(edges, edge_alias);
  
  legion_partition edge_primary = dp.partition_by_color(edges);
  
  legion_partition edge_of_ghost_cell = dp.partition_by_image(cells, edges, cell_to_edge, cell_ghost);
  
  legion_partition edge_ghost = dp.partition_by_difference(edges, edge_of_ghost_cell, edge_primary);
  
  legion_partition edge_of_shared_cell = dp.partition_by_image(cells, edges, cell_to_edge, cell_shared);
  
  legion_partition edge_shared = dp.partition_by_intersection(edges, edge_of_shared_cell, edge_primary);
  
  legion_partition edge_exclusive = dp.partition_by_difference(edges, edge_primary, edge_shared); */
  
  dp.output_partition(cells, cell_primary, cell_ghost, cell_shared, cell_exclusive);
  
  dp.output_partition(vertices, vertex_primary, vertex_ghost, vertex_shared, vertex_exclusive);
  
//  dp.output_partition(edges, edge_primary, edge_ghost, edge_shared, edge_exclusive);
}

} // namespace execution
} // namespace flecsi
