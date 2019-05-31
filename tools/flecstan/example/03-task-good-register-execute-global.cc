
// Compiles and runs.

#include "execution/execution.h"

// No namespace
void
foo() {}

// Task registration
flecsi_register_task(foo, , loc, single); // hash("::foo");

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Task execution
  flecsi_execute_task(foo, , single); // hash("::foo");
}
} // namespace execution
} // namespace flecsi
