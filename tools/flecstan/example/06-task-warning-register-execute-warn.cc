
// Compiles and runs, but a warning might be appropriate.

#include "execution/execution.h"

void
foo() {
  std::cout << "::foo" << std::endl;
}
namespace ns {
void
foo() {
  std::cout << "ns::foo" << std::endl;
}

// Registers ns::foo, not ::foo, in spite of the hash
flecsi_register_task(foo, , loc, single); // hash("::foo")
} // namespace ns

namespace flecsi {
namespace execution {
void
driver(int, char **) {
  // Matches the registered hash and calls ns::foo,
  // but could the user have wanted ::foo?
  flecsi_execute_task(foo, , single); // hash("::foo")
}
} // namespace execution
} // namespace flecsi
