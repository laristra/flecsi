
// Error (run time segfault).

#include "execution/execution.h"

namespace ns {
void
foo() {}

// Task registration.
// This is another example of somebody doing
// something goofy (a blank namespace argument),
// but it does compile. The end result (run time
// segfault) is the same as what would happen
// if the namespace had a typo.
flecsi_register_task(foo, , loc, single); // hash("::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Task execution.
  flecsi_execute_task(foo, ns, single); // hash("ns::foo")
}
} // namespace execution
} // namespace flecsi
