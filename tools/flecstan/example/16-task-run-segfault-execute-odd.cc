
// Error (run time segfault).

#include "execution/execution.h"

namespace ns {
void
foo() {}

// hash("ns::foo")
flecsi_register_task(foo, ns, loc, single); // hash("ns::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // The following is odd, but it compiles.
  // It's perhaps unlikely that somebody
  // would write this. Then again, users
  // are good at doing unexpected things. :-)
  flecsi_execute_task(ns::foo, , single); // hash("::ns::foo")
}
} // namespace execution
} // namespace flecsi
