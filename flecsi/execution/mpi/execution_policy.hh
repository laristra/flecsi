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
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif 

#include "flecsi/runtime/backend.hh"
#include <flecsi/execution/mpi/reduction_wrapper.hh>
#include <flecsi/utils/flog.hh>

namespace flecsi {
namespace execution {

//--------------------------------------------------------------------------//
// Task interface.
//--------------------------------------------------------------------------//

/*!
  MPI backend task registration. For documentation on this
  method please see task::register_task.
 */

template<size_t TASK,
  typename RETURN,
  typename ARG_TUPLE,
  RETURN (*DELEGATE)(ARG_TUPLE)>
bool
register_task(processor_type_t processor, launch_t launch, std::string name) {
  return context_t::instance()
    .template register_function<TASK, RETURN, ARG_TUPLE, DELEGATE>();
} // register_task

//--------------------------------------------------------------------------//
// Reduction interface.
//--------------------------------------------------------------------------//

template<size_t HASH, typename TYPE>
using reduction_wrapper = mpi::reduction_wrapper<HASH, TYPE>;

} // namespace execution
} // namespace flecsi
