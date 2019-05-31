
// Compiles and runs.

#include "execution/execution.h"

namespace A {
// Let's register a task that returns a value.
int
foo() {
  return 123;
}
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // The task's return value is passed back.
  int bar = flecsi_execute_task(foo, A, single);
  std::cout << bar << std::endl; // prints 123
}
} // namespace execution
} // namespace flecsi
