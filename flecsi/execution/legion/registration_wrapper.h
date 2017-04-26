/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_registration_wrapper_h
#define flecsi_execution_legion_registration_wrapper_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 14, 2017
//----------------------------------------------------------------------------//

#include <legion.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The registration_wrapper__ type selects between void and non-void
//! return values for task registration.
//!
//! @tparam RETURN The return type of the task.
//! @tparam TASK   The function pointer template type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  RETURN (*TASK)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__
{
  //--------------------------------------------------------------------------//
  //! This method registers the given task with the Legion runtime.
  //!
  //! @tparam ARGS The variadic argument pack.
  //--------------------------------------------------------------------------//

  template<typename ... ARGS>
  static void register_task(ARGS && ... args) {
    Legion::HighLevelRuntime::register_legion_task<RETURN, TASK>(
      std::forward<ARGS>(args) ...);
  } // register_task
}; // struct registration_wrapper__

//----------------------------------------------------------------------------//
//! Partial specialization of registration_wrapper__ for void return type.
//!
//! @tparam TASK   The function pointer template type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  void (*TASK)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__<void, TASK>
{
  template<typename ... ARGS>
  static void register_task(ARGS && ... args) {
    Legion::HighLevelRuntime::register_legion_task<TASK>(
      std::forward<ARGS>(args) ...);
  } // register_task
}; // struct registration_wrapper__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
