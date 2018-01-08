#include<flecsi/execution/execution.h>

#include<specialization/mesh/tasks.h>

namespace flecsi {
namespace tutorial {

flecsi_register_task(initialize_mesh, flecsi::tutorial, loc, single);

} // namespace tutorial
} // namespace flecsi
