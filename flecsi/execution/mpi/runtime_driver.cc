/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <cstddef>
#include <cstdint>

#include "flecsi/data/data.h"
#include "flecsi/execution/mpi/runtime_driver.h"

#include "flecsi/execution/context.h"
#include "flecsi/coloring/coloring_types.h"

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
remap_shared_entities()
{
  auto& flecsi_context = context_t::instance();
  const int my_color = flecsi_context.rank;

  for (auto coloring_info_pair : flecsi_context.coloring_info_map()) {

    //auto &coloring_info = flecsi_context.coloring_info(index_space);
    auto index_space = coloring_info_pair.first;
    auto coloring_info = coloring_info_pair.second;

    //std::cout << "data handle index space: " << h.index_space << std::endl;
    auto &my_coloring_info = flecsi_context.coloring_info(index_space).at(
      my_color);
    auto index_coloring = flecsi_context.coloring(index_space);

    size_t index = 0;
    for (auto shared : index_coloring.shared) {
      clog_rank(warn, 0) << "myrank: " << my_color
                         << " shared id: " << shared.id
                         << ", rank: " << shared.rank
                         << ", offset: " << shared.offset
                         << ", index: " << index << std::endl;
      for (auto peer : shared.shared) {
        MPI_Send(&index, 1, MPI_UNSIGNED_LONG_LONG, peer, 77, MPI_COMM_WORLD);
      }
      index++;
    }

    std::vector<size_t> ghost_index(index_coloring.ghost.size());

    MPI_Status status;
    std::set<flecsi::coloring::entity_info_t> new_ghost;

    for (auto ghost : index_coloring.ghost) {
      //index = 0;
      MPI_Recv(&index, 1, MPI_UNSIGNED_LONG_LONG,
               ghost.rank, 77, MPI_COMM_WORLD, &status);
      new_ghost.insert(
        flecsi::coloring::entity_info_t(ghost.id, ghost.rank, index, {}));
      //clog(warn) << "index: " << index << std::endl;
    }
    for (auto ghost : index_coloring.ghost) {
      clog_rank(warn, 1) << "myrank: " << my_color
                         << " old ghost id: " << ghost.id
                         << ", rank: " << ghost.rank
                         << ", offset: " << ghost.offset
                         << std::endl;
    }
    for (auto ghost : new_ghost) {
      clog_rank(warn, 1) << "myrank: " << my_color
                         << " new ghost id: " << ghost.id
                         << ", rank: " << ghost.rank
                         << ", offset: " << ghost.offset
                         << std::endl;
    }
    context_t::instance().coloring(index_space).ghost.swap(new_ghost);
    //context_t ::instance().set_coloring(index_space, new_ghost);
  }
}

void
runtime_driver(
  int argc,
  char ** argv
)
{
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "In MPI runtime driver" << std::endl;
  }

#if defined(FLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization tlt task" << std::endl;
  }

  // Execute the specialization driver.
  specialization_tlt_init(argc, argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT

  // Register user data, invokes callbacks to add field info into context
  data::storage_t::instance().register_all();

  // allocate storage for data.
  remap_shared_entities();

  // Execute the user driver.
  driver(argc, argv);

} // runtime_driver

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
