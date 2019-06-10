
// Compiles and runs, but a warning might be appropriate.

#include "execution/execution.h"

namespace A {
void
foo() {}
} // namespace A

namespace B {
void
foo() {}
// Registers B::foo, but with hash A::foo
flecsi_register_task(foo, A, loc, single); // hash("A::foo")
} // namespace B

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Executes B::foo (not A::foo, as one might believe)
  flecsi_execute_task(foo, A, single); // hash("A::foo")
}
} // namespace execution
} // namespace flecsi
