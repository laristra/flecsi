
// Compiles and runs.

#include "execution/execution.h"

namespace ns {
void
foo() {}

// Task registration (inside namespace)
flecsi_register_task(foo, ns, loc, single); // hash("ns::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Task execution
  flecsi_execute_task(foo, ns, single); // hash("ns::foo")
}
} // namespace execution
} // namespace flecsi
