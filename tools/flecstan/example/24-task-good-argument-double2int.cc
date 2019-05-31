
// Compiles and runs.

#include "execution/execution.h"

namespace A {
// Need an int.
void
foo(int) {}
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Send a double.
  // Works fine; tuple<double> converts to tuple<int>.
  // I might have expected a warning about conversion.
  flecsi_execute_task(foo, A, single, 1.2345);
}
} // namespace execution
} // namespace flecsi
