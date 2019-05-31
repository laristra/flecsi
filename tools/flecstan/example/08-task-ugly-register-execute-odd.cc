
// Compiles and runs, but is perhaps a bit goofy.

#include "execution/execution.h"

namespace ns {
void
foo() {}

// hash("::ns" + "::" + "foo")  ==>  hash("::ns::foo")
flecsi_register_task(foo, ::ns, loc, single);
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // hash("" + "::" + "ns::foo")  ==>  hash("::ns::foo")
  flecsi_execute_task(ns::foo, , single);
}
} // namespace execution
} // namespace flecsi
