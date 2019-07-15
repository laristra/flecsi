/*
# Incorrect type, data model, and range
flecsi_register_field(mesh_t, example, pressure, double, dense, 2, cells);
auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
auto f = flecsi_get_handle(m, example, pressure, int, sparse, 4);
*/

#include <flecsi-tutorial/specialization/mesh/mesh.h>
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <iostream>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_field(mesh_t, example, pressure, double, dense, 2, cells);

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {
  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto f = flecsi_get_handle(m, example, pressure, int, sparse, 4);
}

} // namespace execution
} // namespace flecsi
