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

namespace flecsi {
namespace execution {

/*!
 The legion_runtime_state_t type provides storage for Legion runtime
 information that can be reinitialized as needed to store const
 data types and references as required by the Legion runtime.

 @ingroup legion-execution
 */

struct legion_runtime_state_t {

  legion_runtime_state_t(Legion::Context & context_,
    Legion::Runtime * runtime_,
    const Legion::Task * task_,
    const std::vector<Legion::PhysicalRegion> & regions_)
    : context(context_), runtime(runtime_), task(task_), regions(regions_) {}

  Legion::Context & context;
  Legion::Runtime * runtime;
  const Legion::Task * task;
  const std::vector<Legion::PhysicalRegion> & regions;

}; // struct legion_runtime_state_t

class flecsi_task_base_t
{
protected:
  legion_runtime_state_t state_;

}; // class flecsi_task_base_t

} // namespace execution
} // namespace flecsi
