
// Error (compile time).

#include "execution/execution.h"

namespace A {
// This task takes 1 argument...
void
foo(const int i) {}
flecsi_register_task(foo, A, loc, single);
} // namespace A

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // So, what if we send 0 arguments?
  // Clang prints this error as:
  //    error: no viable conversion from 'tuple<(no argument)>'
  //    to 'tuple<int>'
  // That's reasonable, but perhaps our FleCSI Static Analyzer
  // could print something that's just a bit more clear.
  flecsi_execute_task(foo, A, single);
}
} // namespace execution
} // namespace flecsi
