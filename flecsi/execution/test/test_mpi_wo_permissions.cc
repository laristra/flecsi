/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

clog_register_tag(coloring);

namespace flecsi {
namespace test_mesh {

//----------------------------------------------------------------------------//
// Type definitions and client registration
//----------------------------------------------------------------------------//

using mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<size_t PS>
using mesh_handle_t = data_client_handle_u<mesh_t, PS>;

flecsi_register_data_client(mesh_t, test_mesh, mesh1);

} // namespace test_mesh
} // namespace flecsi

namespace flecsi {
namespace test_task {

//----------------------------------------------------------------------------//
// Task definitions and registration
//----------------------------------------------------------------------------//
void
traverse_mesh_ro(test_mesh::mesh_handle_t<ro> m) {
  std::cout << "\n\nIn traverse mesh task with read only handle permissions"
            << std::endl;

  // Iterate over the vertices
  for(auto v : m.vertices())
    std::cout << "vertex id: " << v->id() << std::endl;

  // Iterate over the cells
  for(auto c : m.cells())
    std::cout << "cell id: " << c->id() << std::endl;

} // traverse_mesh

void
traverse_mesh_wo(test_mesh::mesh_handle_t<wo> m) {
  std::cout << "\n\nIn traverse mesh task with write only handle permissions"
            << std::endl;

  // Iterate over the vertices
  for(auto v : m.vertices())
    std::cout << "vertex id: " << v->id() << std::endl;

  // Iterate over the cells
  for(auto c : m.cells())
    std::cout << "cell id: " << c->id() << std::endl;

} // traverse_mesh

flecsi_register_task(traverse_mesh_ro, flecsi::test_task, loc, single);
flecsi_register_task(traverse_mesh_wo, flecsi::test_task, loc, single);

} // namespace test_task
} // namespace flecsi

namespace flecsi {
namespace execution {
//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  std::cout << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();
} // specialization_tlt_init

void
specialization_spmd_init(int argc, char ** argv) {
  std::cout << "In specialization spmd init" << std::endl;
  auto mh =
    flecsi_get_client_handle(flecsi::test_mesh::mesh_t, test_mesh, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  std::cout << "In driver" << std::endl;
  auto mh =
    flecsi_get_client_handle(flecsi::test_mesh::mesh_t, test_mesh, mesh1);
  flecsi_execute_task(traverse_mesh_ro, flecsi::test_task, single, mh);
  flecsi_execute_task(traverse_mesh_wo, flecsi::test_task, single, mh);
} // scope

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(test_mpi_wo_permissions, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
