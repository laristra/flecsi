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

#include <flecsi/runtime/mpi/context.hh>
#include <flecsi/runtime/program_options.hh>

using namespace boost::program_options;

namespace flecsi::runtime {

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
} // initialize

//----------------------------------------------------------------------------//
// Implementation of context_t::finalize.
//----------------------------------------------------------------------------//

void
context_t::finalize() {

  context::finalize_generic();

#ifndef GASNET_CONDUIT_MPI
  if(context::initialize_dependent_) {
    MPI_Finalize();
  } // if
#endif
} // finalize

//----------------------------------------------------------------------------//
// Implementation of context_t::start.
//----------------------------------------------------------------------------//

int
context_t::start(const std::function<int(int, char **)> & action) {

  /*
    Register reduction operations.
   */

  for(auto ro : reduction_registry_) {
    ro();
  } // for

  context::threads_per_process_ = 1;
  context::threads_ = context::processes_;

  std::vector<char *> largv;
  largv.push_back(argv_[0]);

  for(auto opt = unrecognized_options_.begin();
      opt != unrecognized_options_.end();
      ++opt) {
    largv.push_back(opt->data());
  } // for

  return action(largv.size(), largv.data());
}

} // namespace flecsi::runtime
