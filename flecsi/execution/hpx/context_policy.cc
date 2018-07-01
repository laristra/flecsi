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

#if !defined(ENABLE_HPX)
  #error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <flecsi/execution/hpx/context_policy.h>

namespace flecsi {
namespace execution {

hpx_context_policy_t::hpx_context_policy_t()
  : min_reduction_{0}
  , max_reduction_{0}
{
#if defined(_MSC_VER)
  hpx::detail::init_winsocket();
#endif
}

// Return the color for which the context was initialized.
std::size_t hpx_context_policy_t::color() const
{
  return hpx::get_locality_id();
}

// Main HPX thread, does nothing but wait for the application to exit
int
hpx_context_policy_t::hpx_main(
    int (*driver)(int, char * []),
    int argc,
    char * argv[]) {

  // initialize executors (possible only after runtime is active)
  exec_ = hpx::threads::executors::pool_executor{"default"};
  mpi_exec_ = hpx::threads::executors::pool_executor{"mpi"};

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

  // Create the resource partitioner
  std::vector<std::string> const cfg = {
      // allocate at least two cores
      "hpx.force_min_os_threads!=2",
      // make sure hpx_main is always executed
      "hpx.run_hpx_main!=1",
      // allow for unknown command line options
      "hpx.commandline.allow_unknown!=1",
      // disable HPX' short options
      "hpx.commandline.aliasing!=0"
  };

  hpx::resource::partitioner rp{
      hpx::util::bind_front(&hpx_context_policy_t::hpx_main, this, driver),
      argc, argv, cfg};

  // Create a thread pool encapsulating the default scheduler
  rp.create_thread_pool("default", hpx::resource::local_priority_fifo);

  // Create a thread pool for executing MPI tasks
  rp.create_thread_pool("mpi", hpx::resource::static_);

  // Add first core to mpi pool
  rp.add_resource(rp.numa_domains()[0].cores()[0].pus()[0], "mpi");

  // Now, initialize and run the HPX runtime, will return when done.
  return hpx::init();
}

} // namespace execution
} // namespace flecsi

