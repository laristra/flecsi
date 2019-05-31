
// Error (compile time).

#include "execution/execution.h"

namespace ns {
void
foo() {}
} // namespace ns

// You can't force the register macro to see the namespace in front
// by explicitly providing it, because the macro's first argument
// is used (via ##) to build names. The "::" foils this. So, either
// use the macro in the namespace (as shown in another example), or
// outside the namespace and with a using (also shown elsewhere).
flecsi_register_task(ns::foo, , loc, single);

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  flecsi_execute_task(foo, ns, single);
}
} // namespace execution
} // namespace flecsi
