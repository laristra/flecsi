/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_task_h
#define flecsi_execution_task_h

#include <iostream>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/utils/static_verify.h"

///
/// \file task.h
/// \authors bergen
/// \date Initial file creation: Jul 26, 2016
///

namespace flecsi {
namespace execution {

///
/// \struct task__ task.h
/// \brief task__ provides...
///
template<typename execution_policy_t>
struct task__
{

  ///
  /// Register a user task with the FleCSI runtime.
  ///
  /// \tparam R The return type of the user task.
  /// \tparam A A std::tuple of the user task arguments.
  ///
  /// \param address The address of the user task.
  /// \param processor The processor type for task execution.
  /// \param launch The launch mode for task execution.
  ///
  /// \return The return type for task registration is determined by
  ///         the specific backend runtime being used.
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
  /// Execute a registered task.
  ///
  /// \tparam R The return type of the task.
  /// \tparam T The user task handle type.
  /// \tparam As The task arguments.
  ///
  /// \param address A unique identifier used to lookup the task
  ///                in the task registry.
  /// \param processor The processor type on which to execute the task.
  /// \param launch The launch mode for the task.
  /// \param parent A hash key that uniquely identifies the calling task.
  /// \param user_task_handle The user task handle.
  /// \param args The arguments to pass to the user task during execution.
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
    size_t parent,
    T user_task_handle,
    As &&... args
  )
  {
    return execution_policy_t::template execute_task<R>(
      task_hash_t::make_key(address, processor, launch), parent,
      user_task_handle, std::forward<As>(args)...);
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

/// Static verification of public future interface for type defined by
/// execution policy.
namespace verify_future {

FLECSI_MEMBER_CHECKER(wait);
FLECSI_MEMBER_CHECKER(get);

static_assert(verify_future::has_member_wait<future__<double>>::value,
  "future type missing wait method");

static_assert(verify_future::has_member_get<future__<double>>::value,
  "future type missing get method");

} // namespace verify_future

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
