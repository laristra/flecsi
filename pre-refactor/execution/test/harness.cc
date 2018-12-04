/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchlog.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <flecsi/execution/test/harness.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {

  {
  clog_tag_guard(harness);
  clog(info) << "specialization_tlt_init function" << std::endl;
  } // scope

  coloring_map_t map { index_spaces::vertices, index_spaces::cells };

  flecsi_execute_mpi_task(add_colorings, flecsi::execution, map);

  auto & context { execution::context_t::instance() };

  auto & vinfo { context.coloring_info(index_spaces::vertices) };
  auto & cinfo { context.coloring_info(index_spaces::cells) };

  adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for(auto & itr : cinfo){
    size_t color{itr.first};
    const coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  {
  clog_tag_guard(harness);
  clog(info) << "Adding adjacency info: " << ai << std::endl;
  } // scope

  context.add_adjacency(ai);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_spmd_init(int argc, char ** argv) {

  {
  clog_tag_guard(harness);
  clog(info) << "specialization_spmd_init function" << std::endl;
  } // scope

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  flecsi_execute_task(initialize_mesh, flecsi::execution, single, mh);

} // specialization_spmd_ini

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
