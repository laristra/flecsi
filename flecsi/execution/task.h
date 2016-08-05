/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_h
#define flecsi_task_h

#include "flecsi/execution/processor.h"

/*!
 * \file task.h
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

namespace flecsi {
namespace execution {

/*!
  \struct task__ task.h
  \brief task__ provides...
 */
template<typename execution_policy_t>
struct task__
{

  using task_key_t = typename execution_policy_t::task_key_t;

  template<typename R, typename ... Args>
  static decltype(auto) register_task(uintptr_t key, processor_t processor)
    {
      return execution_policy_t::template register_task<R, Args...>(key,
        processor);
    } // register_task

  template<typename ... Args>
  static decltype(auto) execute_task(uintptr_t key, 
                 processor_t processor, Args && ... args)
    {
    return execution_policy_t::execute_task(
                  key, processor, std::forward<Args>(args) ...);
    } // execute

}; // class task

} // namespace execution 
} // namespace flecsi

/*
  This include file defines the flecsi_execution_policy_t used below.
 */
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
