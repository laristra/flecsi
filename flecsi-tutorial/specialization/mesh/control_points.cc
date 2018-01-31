#include <flecsi-config.h>

#include <flecsi/execution/execution.h>
#include <flecsi/execution/context.h>
#include <specialization/mesh/coloring.h>
#include <specialization/mesh/mesh.h>
#include <specialization/mesh/tasks.h>
#include <specialization/mesh/types.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {

  using namespace tutorial;

  coloring_map_t map { index_spaces::vertices, index_spaces::cells };

  flecsi_execute_mpi_task(add_colorings, flecsi::tutorial, map);

  auto & context { execution::context_t::instance() };

  auto & vinfo { context.coloring_info(index_spaces::vertices) };
  auto & cinfo { context.coloring_info(index_spaces::cells) };

  coloring::adjacency_info_t ai;

  // Add adjacency information for cells -> vertices
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for(auto & itr : cinfo){
    size_t color{itr.first};
    const coloring::coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  context.add_adjacency(ai);

  // Add sparse support for cells
  execution::context_t::sparse_index_space_info_t isi;
  isi.max_entries_per_index = 5;
  isi.reserve_chunk = 8192;
  context.set_sparse_index_space_info(index_spaces::cells, isi);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_spmd_init(int argc, char ** argv) {

  using namespace tutorial;

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  flecsi_execute_task(initialize_mesh, flecsi::tutorial, single, mh);

} // specialization_spmd_ini

} // namespace execution
} // namespace flecsi
