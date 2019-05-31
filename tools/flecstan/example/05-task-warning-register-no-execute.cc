
// Compiles and runs, but a warning might be appropriate.

#include "execution/execution.h"

namespace ns {
void
foo() {}
flecsi_register_task(foo, ns, loc, single);
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // If there's no execution, anywhere, of a registered task,
  // then perhaps the FleCSI Static Analyzer should print a
  // warning. This would be the analog of a compiler's "variable
  // defined but never used" warning.
}
} // namespace execution
} // namespace flecsi
