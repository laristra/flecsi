
// Error (compile time).

#include "execution/execution.h"

namespace ns {
void
foo() {}
} // namespace ns

// Compile time error: foo is not in scope.
// The register macro does *not* place namespace:: in front!
flecsi_register_task(foo, ns, loc, single);

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Task execution
  flecsi_execute_task(foo, ns, single);
}
} // namespace execution
} // namespace flecsi
