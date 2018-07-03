/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: May 4, 2017
///

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>

#define INDEX_ID 0
#define VERSIONS 1

clog_register_tag(gid_to_lid);

class client_type : public flecsi::data::data_client_t {
public:
  using type_identifier_t = flecsi::data::data_client_t;
};

flecsi_register_field(
    client_type,
    name_space,
    cell_ID,
    size_t,
    dense,
    INDEX_ID,
    VERSIONS);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(trace) << "In specialization top-level-task init" << std::endl;

  supplemental::coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;
  flecsi_execute_mpi_task(add_colorings, flecsi::supplemental, map);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
  auto runtime = Legion::Runtime::get_runtime();
  const int my_color = runtime->find_local_MPI_rank();
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
  int my_color;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_color);
#endif

  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  context_t & context_ = context_t::instance();

  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map =
      context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  std::map<size_t, size_t> gid_to_lid_map;

  auto entries = index_coloring->second.exclusive;

  size_t lid = 0;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end();
       ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[lid++] = entity.id;
  }

  entries = index_coloring->second.shared;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end();
       ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[lid++] = entity.id;
  }

  entries = index_coloring->second.ghost;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end();
       ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[lid++] = entity.id;
  }

  std::map<size_t, size_t> index_map = context_.index_map(INDEX_ID);

  clog_assert(
      gid_to_lid_map == index_map, "global to local ID mapping is incorrect");

} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
