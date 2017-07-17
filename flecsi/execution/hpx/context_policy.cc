/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#if !defined(ENABLE_HPX)
#error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include <flecsi/execution/hpx/context_policy.h>

#include <flecsi/data/storage.h>

namespace flecsi {
namespace execution {

hpx_context_policy_t::hpx_context_policy_t() {
#if defined(_MSC_VER)
  hpx::detail::init_winsocket();
#endif
}

// Main HPX thread, does nothing but wait for the application to exit
int
hpx_context_policy_t::hpx_main(
    int (*driver)(int, char * []),
    int argc,
    char * argv[]) {
  // execute user code (driver)
  int retval = (*driver)(argc, argv);

  // tell the runtime it's ok to exit
  hpx::finalize();

  return retval;
}

int
hpx_context_policy_t::start_hpx(
    int (*driver)(int, char * []),
    int argc,
    char * argv[]) {
  std::vector<std::string> const cfg = {
      // make sure hpx_main is always executed
      "hpx.run_hpx_main!=1",
      // allow for unknown command line options
      "hpx.commandline.allow_unknown!=1",
      // disable HPX' short options
      "hpx.commandline.aliasing!=0"};

  using hpx::util::placeholders::_1;
  using hpx::util::placeholders::_2;
  hpx::util::function_nonser<int(int, char **)> start_function =
      hpx::util::bind(&hpx_context_policy_t::hpx_main, this, driver, _1, _2);

  return hpx::init(start_function, argc, argv, cfg);
}

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
