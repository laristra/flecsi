/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include "context.hh"

using namespace boost::program_options;

namespace flecsi::runtime {

//----------------------------------------------------------------------------//
// Helper function for context_t::initialize.
//----------------------------------------------------------------------------//

int
context_t::hpx_main(bool dependent, int argc, char ** argv) {

  // initialize executors (possible only after runtime is active)
  exec_ = hpx::threads::executors::pool_executor{"default"};
  mpi_exec_ = hpx::threads::executors::pool_executor{"mpi"};

  context::process_ = hpx::get_locality_id();
  context::processes_ = hpx::get_num_localities().get();

  auto initialize_status = context::initialize_generic(argc, argv, dependent);

  if(initialize_status != success && dependent) {
    hpx::finalize();
  } // if

  initial_status_set = true;
  initial_stat = static_cast<status>(initialize_status);
  cv.notify_one();

  int ret_val = top_level_action()(argc, argv);

  auto finalize_status = context::finalize_generic();
  final_status_set = true;
  final_stat = static_cast<status>(finalize_status);
  cv.notify_one();

  return hpx::finalize();

  // return ret_val;
}

//----------------------------------------------------------------------------//
// Implementation of context_t::initialize.
//----------------------------------------------------------------------------//

int
context_t::initialize(int argc, char ** argv, bool dependent) {
    // Create the resource partitioner
  std::vector<std::string> const cfg = {// allocate at least two cores
    "hpx.force_min_os_threads!=2",
    // make sure hpx_main is always executed
    "hpx.run_hpx_main!=1",
    // allow for unknown command line options
    "hpx.commandline.allow_unknown!=1",
    // disable HPX' short options
    "hpx.commandline.aliasing!=0"};

  hpx::resource::partitioner rp{
    hpx::util::bind_front(&context_t::hpx_main, this, dependent), argc,
    argv, cfg};

  // Create a thread pool encapsulating the default scheduler
  rp.create_thread_pool("default", hpx::resource::local_priority_fifo);

  // Create a thread pool for executing MPI tasks
  rp.create_thread_pool("mpi", hpx::resource::static_);

  // Add first core to mpi pool
  rp.add_resource(rp.numa_domains()[0].cores()[0].pus()[0], "mpi");

  // Now, initialize and run the HPX runtime.
  hpx::start();

  std::unique_lock<std::mutex> ul(m);
  cv.wait(ul, [this](){ return initial_status_set.load(std::memory_order_relaxed); });

  return initial_stat;
} // initialize

//----------------------------------------------------------------------------//
// Implementation of context_t::finalize.
//----------------------------------------------------------------------------//

int
context_t::finalize() {

  std::unique_lock<std::mutex> ul(m);
  cv.wait(ul, [this](){ return final_status_set.load(std::memory_order_relaxed); });

  hpx::stop();

  return final_stat;

//   auto status = context::finalize_generic();

// #ifndef GASNET_CONDUIT_MPI
//   if(status == success && context::initialize_dependent_) {
//     MPI_Finalize();
//   } // if
// #endif

//   return status;
} // finalize

//----------------------------------------------------------------------------//
// Implementation of context_t::start.
//----------------------------------------------------------------------------//

int
context_t::start() {
  return 0;
}

} // namespace flecsi::runtime
