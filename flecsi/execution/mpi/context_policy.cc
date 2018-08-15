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

#include <flecsi/execution/mpi/context_policy.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of mpi_context_policy_t::initialize.
//----------------------------------------------------------------------------//

int
mpi_context_policy_t::initialize(
  int argc,
  char ** argv
)
{
  MPI_Comm_rank(MPI_COMM_WORLD, &color_);
  MPI_Comm_size(MPI_COMM_WORLD, &colors_);

  runtime_driver(argc, argv);

  return 0;
} // mpi_context_policy_t::initialize

} // namespace execution 
} // namespace flecsi

