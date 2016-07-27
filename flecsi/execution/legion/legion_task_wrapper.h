/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_task_wrapper_h
#define flecsi_legion_task_wrapper_h

/*!
 * \file legion_task_wrapper.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

//template<typename T>
struct legion_task_wrapper_
{
  using lr_context_t = LegionRuntime::HighLevel::Context;
  using lr_runtime_t = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_task_t = LegionRuntime::HighLevel::Task;
  using lr_regions_t =
    std::vector<LegionRuntime::HighLevel::PhysicalRegion>;

  static void execute(const lr_task_t * task, const lr_regions_t & regions,
    lr_context_t context, lr_runtime_t * runtime)
    {
      std::cout << "Hello from task_wrapper" << std::endl;
    } // execute

}; // class legion_task_wrapper_

} // namespace flecsi

#endif // flecsi_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
