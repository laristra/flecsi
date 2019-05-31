
// Error (compile time).

#include "execution/execution.h"

namespace ns {
void
foo() {} // no args
void
foo(int) {} // int arg

// Which "foo" we want is ambiguous here.
flecsi_register_task(foo, ns, loc, single);
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // The no-args foo() is presumably intended here, because
  // no variadic arguments are provided. However, the compiler
  // goes bonkers over the code emitted by the macro. :-(
  flecsi_execute_task(foo, ns, single);
}
} // namespace execution
} // namespace flecsi
