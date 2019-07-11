
/*
# Incorrect data model
flecsi_register_field(mesh_t, example, pressure, double, dense, 1, cells);
auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
auto f = flecsi_get_handle(m, example, pressure, double, sparse, 0);
*/

#include <iostream>
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi-tutorial/specialization/mesh/mesh.h>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_field(mesh_t, example, pressure, double, dense, 1, cells);

namespace flecsi {
namespace execution {

void driver(int argc, char **argv)
{
   auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

   // The error here is "sparse" when we want "dense"
   auto f = flecsi_get_handle(m, example, pressure, double, sparse, 0);
}

}
}
