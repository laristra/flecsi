
// Error (run time segfault).

#include "execution/execution.h"

namespace ns {
void
foo() {}
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Oops, we're trying to execute a task
  // that we didn't register.
  flecsi_execute_task(foo, ns, single);
}
} // namespace execution
} // namespace flecsi
