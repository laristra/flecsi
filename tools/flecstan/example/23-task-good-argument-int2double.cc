
// Compiles and runs.

#include "execution/execution.h"

namespace A {
// Need a double.
void
foo(double) {}
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Send an int.
  // This works fine, because tuple<int>
  // converts to tuple<double>.
  flecsi_execute_task(foo, A, single, 1);
}
} // namespace execution
} // namespace flecsi
