
// Error (compile time).

#include "execution/execution.h"

namespace A {}

namespace B {
void
foo() {}
flecsi_register_task(foo, B, loc, single);
} // namespace B

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Compile time error: A::foo does not exist.
  // The execute macro, unlike the register macro as shown
  // in another example, *does* place namespace:: in front.
  flecsi_execute_task(foo, A, single);
}
} // namespace execution
} // namespace flecsi
