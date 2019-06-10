
// Error (run time FleCSI error).

#include "execution/execution.h"

namespace A {
void
foo() {}
// Registers A::foo, with hash("A::foo")
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace B {
void
foo() {}
// Registers B::foo...but with hash("A::foo")!
// The mistake was probably copying the earlier macro
// invocation, but forgetting to change A to B.
// The run time error occurs here, not in the execute.
flecsi_register_task(foo, A, loc, single);
} // namespace B

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // The run time error occurred above, not here
  flecsi_execute_task(foo, A, single);
}
} // namespace execution
} // namespace flecsi
