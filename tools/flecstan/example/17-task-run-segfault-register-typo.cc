
// Error (run time segfault).

#include "execution/execution.h"

namespace ns {
void
foo() {}

// Task registration.
// Oops, there's a typo in the namespace. This doesn't
// cause a compile-time error here, in the register
// macro, but results in an unintended hash.
flecsi_register_task(foo, typo, loc, single); // hash("typo::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Task execution.
  // Note: had the typo been here, not in the register,
  // it would have caused a compile time error, not a run
  // time error! The difference stems from a difference
  // in how the register and execute macros are defined.
  flecsi_execute_task(foo, ns, single); // hash("ns::foo")
}
} // namespace execution
} // namespace flecsi
