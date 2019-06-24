
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
  // So, what if we send 2 arguments?
  // Clang prints this:
  //    error: no viable conversion from 'tuple<[...],
  //    typename __decay_and_strip<double &>::__type>'
  //    to 'tuple<[...], (no argument)>'
  // For the average user that's sort of goofy, and it's
  // worse than the error we got for too *few* arguments.
  // So, again, there's room for improvement.
  flecsi_execute_task(foo, A, single, 1, 2.34);
}
} // namespace execution
} // namespace flecsi
