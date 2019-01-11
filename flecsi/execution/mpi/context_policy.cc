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
mpi_context_policy_t::initialize(int argc, char ** argv) {
  //--------------------------------------------------------------------------//
  // Set color state data
  //--------------------------------------------------------------------------//

  MPI_Comm_rank(MPI_COMM_WORLD, &color_);
  MPI_Comm_size(MPI_COMM_WORLD, &colors_);

  //--------------------------------------------------------------------------//
  // Add pre-defined MPI ops to reduction map
  //--------------------------------------------------------------------------//

#if 0
  // MPI_MAX
  reduction_ops_[flecsi::utils::const_string_t{"max"}.hash()] = MPI_MAX;

  // MPI_MIN
  reduction_ops_[flecsi::utils::const_string_t{"min"}.hash()] = MPI_MIN;

  // MPI_SUM
  reduction_ops_[flecsi::utils::const_string_t{"sum"}.hash()] = MPI_SUM;

  // MPI_PROD
  reduction_ops_[flecsi::utils::const_string_t{"prod"}.hash()] = MPI_PROD;

  // MPI_LAND
  reduction_ops_[flecsi::utils::const_string_t{"land"}.hash()] = MPI_LAND;

  // MPI_BAND
  reduction_ops_[flecsi::utils::const_string_t{"band"}.hash()] = MPI_BAND;

  // MPI_LOR
  reduction_ops_[flecsi::utils::const_string_t{"lor"}.hash()] = MPI_LOR;

  // MPI_BOR
  reduction_ops_[flecsi::utils::const_string_t{"bor"}.hash()] = MPI_BOR;

  // MPI_LXOR
  reduction_ops_[flecsi::utils::const_string_t{"lxor"}.hash()] = MPI_LXOR;

  // MPI_BXOR
  reduction_ops_[flecsi::utils::const_string_t{"bxor"}.hash()] = MPI_BXOR;

  // MPI_MAXLOC
  reduction_ops_[flecsi::utils::const_string_t{"maxloc"}.hash()] = MPI_MAXLOC;

  // MPI_MINLOC
  reduction_ops_[flecsi::utils::const_string_t{"minloc"}.hash()] = MPI_MINLOC;
#endif

  //--------------------------------------------------------------------------//
  // Invoke the runtime driver
  //--------------------------------------------------------------------------//

  runtime_driver(argc, argv);

  return 0;
} // mpi_context_policy_t::initialize

} // namespace execution
} // namespace flecsi
