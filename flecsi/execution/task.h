/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_task_h
#define flecsi_execution_task_h

#include <iostream>
#include <string>

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
  /// \tparam RETURN The return type of the user task.
  /// \tparam ARG_TUPLE A std::tuple of the user task arguments.
  /// \tparam DELEGATE The delegate function that invokes the user task.
  /// \tparam KEY A hash key identifying the task.
  ///
  /// \param key The \ref task_hash_key_t for the task.
  /// \param name The string identifier of the task.
  ///
  /// \return The return type for task registration is determined by
  ///         the specific backend runtime being used.
  ///
  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE),
    size_t KEY
  >
  static
  decltype(auto)
  register_task(
    task_hash_key_t key,
    std::string name
  )
  {
    return execution_policy_t::template register_task<
      RETURN, ARG_TUPLE, DELEGATE, KEY>(key, name);
  }

  ///
  /// Execute a registered task.
  ///
  /// \tparam RETURN The return type of the task.
  /// \tparam ARGS The task arguments.
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
    typename RETURN,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    ARGS &&... args
  )
  {
    return execution_policy_t::template execute_task<RETURN>(
      key, parent, std::forward<ARGS>(args)...);
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
