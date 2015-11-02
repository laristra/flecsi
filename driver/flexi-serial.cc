/*~-------------------------------------------------------------------------~~*
 * placeholder
 *~-------------------------------------------------------------------------~~*/

#include <execution/task.h>

#define _UTIL_STRINGIFY(s)#s
#define EXPAND_AND_STRINGIFY(s)_UTIL_STRINGIFY(s)
#include EXPAND_AND_STRINGIFY(FLEXI_DRIVER)
#undef _UTIL_STRINGIFY
#undef EXPAND_AND_STRINGIFY

using execution_t = flexi::execution_t<>;

int main(int argc, char ** argv) {
	return execution_t::execute_task(driver);
} // main
