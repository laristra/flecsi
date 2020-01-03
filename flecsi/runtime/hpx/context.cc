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
context_t::hpx_main(int argc, char ** argv) {

  // initialize executors (possible only after runtime is active)
  exec_ = hpx::threads::executors::pool_executor{"default"};
  mpi_exec_ = hpx::threads::executors::pool_executor{"mpi"};

  context::process_ = hpx::get_locality_id();
  context::processes_ = hpx::find_all_localities().size();

  int ret_val = top_level_action()(argc, argv);

  return hpx::finalize();
}

//----------------------------------------------------------------------------//
// Implementation of context_t::initialize.
//----------------------------------------------------------------------------//

int
context_t::initialize(int argc, char ** argv, bool dependent) {
  if(dependent) {
    MPI_Init(&argc, &argv);
  } // if

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  context::process_ = rank;
  context::processes_ = size;

  auto status = context::initialize_generic(argc, argv, dependent);

  if(status != success && dependent) {
    MPI_Finalize();
  } // if

  return status;
}

//----------------------------------------------------------------------------//
// Implementation of context_t::finalize.
//----------------------------------------------------------------------------//

int
context_t::finalize() {
  auto status = context::finalize_generic();

#ifndef GASNET_CONDUIT_MPI
  if(status == success && context::initialize_dependent_) {
    MPI_Finalize();
  } // if
#endif

  return status;
} // finalize

//----------------------------------------------------------------------------//
// Implementation of context_t::start.
//----------------------------------------------------------------------------//

int
context_t::start() {
  /*
    Initialize the runtime arguments
  */
  std::vector<char *> largv;
  largv.push_back(argv_[0]);
  context::threads_per_process_ = 1;

  for(auto opt = unrecognized_options_.begin();
      opt != unrecognized_options_.end();
      ++opt) {
      largv.push_back(opt->data());
  } // for

  context::threads_ = context::processes_ * context::threads_per_process_;

  /*
    Initialize the hpx runtime
  */
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
    hpx::util::bind_front(&context_t::hpx_main, this), largv.size(),
    largv.data(), cfg};

  // Create a thread pool encapsulating the default scheduler
  rp.create_thread_pool("default", hpx::resource::local_priority_fifo);

  // Create a thread pool for executing MPI tasks
  rp.create_thread_pool("mpi", hpx::resource::static_);

  // Add first core to mpi pool
  rp.add_resource(rp.numa_domains()[0].cores()[0].pus()[0], "mpi");

  // Now, initialize and run the HPX runtime.
  return hpx::init();
}

} // namespace flecsi::runtime
