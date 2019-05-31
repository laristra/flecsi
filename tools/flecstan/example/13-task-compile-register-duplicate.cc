
// Error (compile time).

#include "execution/execution.h"

namespace ns {
void
foo() {}

// Duplicate registration.
// Produces a compile time error, because the macro uses
// the task name (via ##) to build the names of a delegate
// function and a bool variable (which are thus duplicated).
flecsi_register_task(foo, ns, loc, single);
flecsi_register_task(foo, ns, loc, single);
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  flecsi_execute_task(foo, ns, single);
}
} // namespace execution
} // namespace flecsi
