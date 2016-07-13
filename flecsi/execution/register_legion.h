/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_register_legion_h
#define flecsi_register_legion_h

/*!
 * \file register_legion.h
 * \authors payne
 * \date Initial file creation: Apr 25, 2016
 */

#include "flecsi/utils/TaskWrapper.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion_execution_policy.h"

namespace flecsi {

/*!
  \class register_legion register_legion.h
  \brief register_legion provides...
 */
template<typename aWrapper_t>
class register_legion
{
public:
  using wrapper_t = aWrapper_t;

  static void cpu_base_impl(const LegionRuntime::HighLevel::Task *taskt,
                            const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                            LegionRuntime::HighLevel::Context ctx, 
                            LegionRuntime::HighLevel::HighLevelRuntime *runtime)
  {
	  wrapper_t* wrapper = (wrapper_t*)(taskt->args);

	  wrapper_t wrapper2(wrapper->handles,wrapper->const_args,
         context_t<legion_execution_policy_t>(1,taskt,regions,ctx,runtime));

	  wrapper2.evaluate();

  }

  static void register_task()
  {
	  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
           <register_legion<wrapper_t>::cpu_base_impl>(wrapper_t::TASK_ID(),
						LegionRuntime::HighLevel::Processor::LOC_PROC,
						wrapper_t::SINGLE/*single*/, wrapper_t::INDEX/*index*/,
	          0,
	          LegionRuntime::HighLevel::TaskConfigOptions(wrapper_t::IS_LEAF),
				    wrapper_t::TASK_NAME());
  }


private:

}; // class register_legion

} // namespace flecsi

#endif // flecsi_register_legion_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
