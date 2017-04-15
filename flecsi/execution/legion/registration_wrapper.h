/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_registration_wrapper_h
#define flecsi_execution_legion_registration_wrapper_h

#include <legion.h>

///
/// \file
/// \date Initial file creation: Apr 14, 2017
///

namespace flecsi {
namespace execution {

///
/// Fix to select between void and non-void return values for Legion problem.
///
template<
  typename RETURN,
  RETURN (*METHOD)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__
{
  template<typename ... ARGS>
  static void register_task(ARGS && ... args) {
    Legion::HighLevelRuntime::register_legion_task<RETURN, METHOD>(
      std::forward<ARGS>(args) ...);
  } // register_task
}; // struct registration_wrapper__

///
/// Partial specialization for void.
///
template<
  void (*METHOD)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__<void, METHOD>
{
  template<typename ... ARGS>
  static void register_task(ARGS && ... args) {
    Legion::HighLevelRuntime::register_legion_task<METHOD>(
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
