
// Error (compile time).

#include "execution/execution.h"

namespace A {
// This task takes 2 arguments...
void
foo(const int i, const double d) {}
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // So, what if we send 1 argument?
  // Clang prints this:
  //    error: no viable conversion from 'tuple<[...], (no argument)>'
  //    to 'tuple<[...], double>'
  // This is another case for which the diagnostic could be improved.
  flecsi_execute_task(foo, A, single, 1);
}
} // namespace execution
} // namespace flecsi
