#include <mpi.h>
#include <unistd.h>

#include <flecsi/supplemental/coloring/add_colorings_dependent_partition.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/execution/legion/dependent_partition.h>
#include <flecsi/io/simple_definition.h>


namespace flecsi {
namespace execution {

void
add_colorings_unified()    
{
  printf("start DP\n");
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  
  int num_cells = sd.num_entities(1);
  int num_vertices = sd.num_entities(0);

  dependent_partition dp;
  legion_entity cells = dp.load_cell(num_cells);
  
  legion_adjacency cell_to_cell = dp.load_cell_to_cell(cells);
  
  legion_partition cell_primary = dp.partition_by_color(cells);
  
  legion_partition cell_closure = dp.partition_by_image(cells, cells, cell_to_cell, cell_primary);
  
  legion_partition cell_ghost = dp.partition_by_difference(cells, cell_closure, cell_primary);
  
  legion_partition cell_ghost_closure = dp.partition_by_image(cells, cells, cell_to_cell, cell_ghost);
  
  legion_partition cell_shared = dp.partition_by_intersection(cells, cell_ghost_closure, cell_primary);
  
  legion_partition cell_exclusive = dp.partition_by_difference(cells, cell_primary, cell_shared);
  
  dp.output_partition(cells, cell_primary, cell_ghost, cell_shared, cell_exclusive);
  
  legion_entity vertices = dp.load_non_cell(num_vertices, 1);
  
  legion_adjacency cell_to_vertex = dp.load_cell_to_others(cells, vertices);
  
  legion_partition vertex_alias = dp.partition_by_image(cells, vertices, cell_to_vertex, cell_primary);
  
  dp.min_reduction_color(vertices, vertex_alias);
  
  legion_partition vertex_primary = dp.partition_by_color(vertices);
  
  dp.output_partition(vertices, vertex_primary, vertex_primary, vertex_primary, vertex_primary);
  
}

} // namespace execution
} // namespace flecsi