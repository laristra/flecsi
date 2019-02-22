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
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not inlcude this file directly!
#else
#include <flecsi/execution/context.h>
#include <flecsi/execution/mpi/reduction_wrapper.h>
#include <flecsi/utils/flog.h>
#endif

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

/*!
  The mpi_execution_policy_t is the backend runtime execution policy
  for MPI.

  @ingroup mpi-execution
 */

struct mpi_execution_policy_t {

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    MPI backend task registration. For documentation on this
    method please see task_u::register_task.
   */

  template<size_t TASK,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool
  register_task(processor_type_t processor, launch_t launch, std::string name) {
    return context_t::instance()
      .template register_function<TASK, RETURN, ARG_TUPLE, DELEGATE>();
  } // register_task

  //--------------------------------------------------------------------------//
  // Reduction interface.
  //--------------------------------------------------------------------------//

  template<size_t HASH, typename TYPE>
  using reduction_wrapper_u = mpi::reduction_wrapper_u<HASH, TYPE>;

}; // struct mpi_execution_policy_t

} // namespace execution
} // namespace flecsi
