/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

/*! @file */

#include <flecsi-config.h>

#if !defined(ENABLE_HPX)
#error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <flecsi/data/storage.h>
#include <flecsi/execution/hpx/context_policy.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of hpx_context_policy_t.
//----------------------------------------------------------------------------//

hpx_context_policy_t::hpx_context_policy_t() {
#if defined(_MSC_VER)
  hpx::detail::init_winsocket();
#endif
} // namespace execution

// Return the color for which the context was initialized.
size_t
hpx_context_policy_t::color() const {
  return hpx::get_locality_id();
}

// Main HPX thread, does nothing but wait for the application to exit
int
hpx_context_policy_t::hpx_main(void (*driver)(int, char *[]),
  int argc,
  char * argv[]) {

  // execute user code (driver)
  (*driver)(argc, argv);

  // tell the runtime it's ok to exit
  return hpx::finalize();
}

int
hpx_context_policy_t::start_hpx(void (*driver)(int, char *[]),
  int argc,
  char * argv[]) {

  // Create the resource partitioner
  std::vector<std::string> cfg = {// allocate at least two cores
    "hpx.force_min_os_threads!=2",
    // make sure hpx_main is always executed
    "hpx.run_hpx_main!=1",
    // allow for unknown command line options
    "hpx.commandline.allow_unknown!=1",
    // disable HPX' short options
    "hpx.commandline.aliasing!=0"};

  // Now, initialize and run the HPX runtime, will return when done.
#if HPX_VERSION_FULL < 0x010500
  return hpx::init(
    hpx::util::bind_front(&hpx_context_policy_t::hpx_main, this, driver), argc,
    argv, cfg);
#else
  // Newer versions of HPX do not allow to explicitly initialize the
  // resource partitioner anymore
  hpx::init_params params;
  params.cfg = std::move(cfg);

  return hpx::init(
    hpx::util::bind_front(&hpx_context_policy_t::hpx_main, this, driver), argc,
    argv, params);
#endif
}

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
