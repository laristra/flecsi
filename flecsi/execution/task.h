/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_h
#define flecsi_task_h

#include <iostream>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/utils/static_verify.h"

///
// \file task.h
// \authors bergen
// \date Initial file creation: Jul 26, 2016
///

namespace flecsi {
namespace execution {

///
// \struct task__ task.h
// \brief task__ provides...
///
template<typename execution_policy_t>
struct task__
{

  // FIXME: Finish Doxygen

  ///
  //
  ///
  template<
    typename R,
    typename A
  >
  static
  decltype(auto)
  register_task(
    uintptr_t address,
    processor_t processor,
    launch_t launch
  )
  {
    return execution_policy_t::template register_task<R, A>(
      task_hash_t::make_key(address, processor, launch));
  } // register_task

  ///
  // Execute a registered task.
  //
  // \tparam R The return type of the task.
  // \tparam T FIXME: This needs to be a handle
  // \tparam As The task arguments.
  //
  // \param address A unique identifier used to lookup the task
  //                in the task registry.
  // \param processor The processor type on which to execute the task.
  // \param launch The launch mode for the task.
  ///
  template<
    typename R,
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_task(
    uintptr_t address,
    processor_t processor,
    launch_t launch,
    T user_task,
    As ... args
  )
  {
    auto targs = std::make_tuple(args ...);
    return execution_policy_t::template execute_task<R>(
      task_hash_t::make_key(address, processor,launch), user_task, targs);
  } // execute

}; // class task

} // namespace execution 
} // namespace flecsi

//
// This include file defines the flecsi_execution_policy_t used below.
//
#include "flecsi_runtime_execution_policy.h"

namespace flecsi {
namespace execution {

using task_t = task__<flecsi_execution_policy_t>;

// Use the execution policy to define the future type.
template<typename R>
using future__ = flecsi_execution_policy_t::future__<R>;

// Static verification of future interface for type defined by
// execution policy.
namespace verify_future {

FLECSI_MEMBER_CHECKER(wait);
FLECSI_MEMBER_CHECKER(get);
FLECSI_MEMBER_CHECKER(set);

static_assert(verify_future::has_member_wait<future__<double>>::value,
  "future type missing wait method");

static_assert(verify_future::has_member_get<future__<double>>::value,
  "future type missing get method");

static_assert(verify_future::has_member_set<future__<double>>::value,
  "future type missing set method");

} // namespace verify_future

} // namespace execution 
} // namespace flecsi

#endif // flecsi_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
