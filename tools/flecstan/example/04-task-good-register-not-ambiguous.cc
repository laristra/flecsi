
// Compiles and runs.

#include "execution/execution.h"

void
foo() {} // foo #1
namespace ns {
void
foo() {} // foo #2

// In this case there's no ambiguity; the foo()
// function in the current namespace is preferred.
flecsi_register_task(foo, ns, loc, single); // hash("ns::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  flecsi_execute_task(foo, ns, single); // hash("ns::foo")
}
} // namespace execution
} // namespace flecsi
