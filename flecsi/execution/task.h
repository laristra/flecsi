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
  //
  ///
  template<
    typename R,
    typename ... As
  >
  static
  decltype(auto)
  execute_task(
    uintptr_t address,
    processor_t processor,
    launch_t launch,
    As ... args
  )
  {
    auto targs = std::make_tuple(args ...);
    return execution_policy_t::template execute_task<R>(
      task_hash_t::make_key(address, processor,launch), targs);
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

} // namespace execution 
} // namespace flecsi

#endif // flecsi_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
