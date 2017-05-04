/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_task_h
#define flecsi_execution_task_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <iostream>
#include <string>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/utils/static_verify.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The task__ type provides a high-level task interface that is
//! implemented by the given execution policy.
//!
//! @tparam EXECUTION_POLICY The backend execution policy.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<
  typename EXECUTION_POLICY
>
struct task__
{

  //--------------------------------------------------------------------------//
  //! Register a user task with the FleCSI runtime.
  //!
  //! @tparam RETURN The return type of the user task.
  //! @tparam ARG_TUPLE A std::tuple of the user task arguments.
  //! @tparam DELEGATE The delegate function that invokes the user task.
  //! @tparam KEY A hash key identifying the task.
  //!
  //! @param key The \ref task_hash_key_t for the task.
  //! @param name The string identifier of the task.
  //!
  //! @return The return type for task registration is determined by
  //!         the specific backend runtime being used.
  //--------------------------------------------------------------------------//

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
    return EXECUTION_POLICY::template register_task<
      RETURN, ARG_TUPLE, DELEGATE, KEY>(key, name);
  } // register_task

  //--------------------------------------------------------------------------//
  //! Execute a registered task.
  //!
  //! @tparam RETURN The return type of the task.
  //! @tparam ARGS The task arguments.
  //!
  //! @param key A \ref task_hash_t key that uniquely identifies the
  //!            calling task.
  //! @param parent A hash key that uniquely identifies the calling task.
  //! @param args The arguments to pass to the user task during execution.
  //--------------------------------------------------------------------------//

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
    return EXECUTION_POLICY::template execute_task<RETURN>(
      key, parent, std::forward<ARGS>(args) ...);
  } // execute_task

}; // struct task__

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_EXECUTION_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi_runtime_execution_policy.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The task_t type is the high-level interface to the FleCSI task model.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

using task_t = task__<FLECSI_RUNTIME_EXECUTION_POLICY>;

//----------------------------------------------------------------------------//
//! Use the execution policy to define the future type.
//!
//! @tparam RETURN The return type of the associated task.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<typename RETURN>
using future__ = FLECSI_RUNTIME_EXECUTION_POLICY::future__<RETURN>;

//----------------------------------------------------------------------------//
// Static verification of public future interface for type defined by
// execution policy.
//----------------------------------------------------------------------------//

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
