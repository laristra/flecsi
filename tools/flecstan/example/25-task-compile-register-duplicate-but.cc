
// Error (compile time).

#include "execution/execution.h"

namespace A {
void
foo() {}
} // namespace A
namespace B {
void
foo() {}
} // namespace B

using A::foo;
using B::foo;
// Several compile-time errors occur here, all essentially
// due to the ambiguity of "foo" (which the registration
// macro doesn't always qualify with the given namespace.
flecsi_register_task(foo, A, loc, single);
flecsi_register_task(foo, B, loc, single);

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  flecsi_execute_task(foo, A, single);
  flecsi_execute_task(foo, B, single);
}
} // namespace execution
} // namespace flecsi
