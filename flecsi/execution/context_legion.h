/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_context_legion_h
#define flecsi_context_legion_h

#include <legion.h>


#include "flecsi/execution/context.h"

/*!
 * \file context_legion.h.h
 * \authors payne
 * \date Initial file creation: Feb 22, 2016
 */

namespace flecsi {
class legion_execution_policy_t;
/*!
  \class context_t context_legion.h.h
  \brief context_t provides...
 */
template<>
class context_t<flecsi::legion_execution_policy_t>
{
public:
 enum class call_state_t : size_t {
   driver = 0,
   task
 }; // enum class call_state_t

 static context_t<legion_execution_policy_t> & instance(
		 LegionRuntime::HighLevel::Context _ctx_l,
		 LegionRuntime::HighLevel::HighLevelRuntime* _rt,
		 const LegionRuntime::HighLevel::Task *_task,
		 const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &_regions)
 {
   static context_t<legion_execution_policy_t> ctx(_ctx_l,_rt,_task,_regions);
   return ctx;
 } // instance
 call_state_t current()
 {
   return call_state_ > 0 ? call_state_t::driver : call_state_t::task;
 } // current

 call_state_t entry() { return static_cast<call_state_t>(++call_state_); }
 call_state_t exit() { return static_cast<call_state_t>(--call_state_); }
 //! Copy constructor (disabled)
 context_t(const context_t &) = delete;

 //! Assignment operator (disabled)
 context_t & operator=(const context_t &) = delete;

private:
 // Constructor
 context_t<legion_execution_policy_t>(
		 LegionRuntime::HighLevel::Context _ctx_l,
		 LegionRuntime::HighLevel::HighLevelRuntime* _rt,
		 const LegionRuntime::HighLevel::Task *_task,
		 const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &_regions) :
				 call_state_(static_cast<size_t>(call_state_t::driver)),
				 ctx_l(_ctx_l),rt(_rt),task(_task),regions(_regions){}


 ~context_t<legion_execution_policy_t>() {}
 size_t call_state_;

 LegionRuntime::HighLevel::Context ctx_l;
 LegionRuntime::HighLevel::HighLevelRuntime* rt;
 const LegionRuntime::HighLevel::Task *task;
 const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions;



}; // class context_t

typedef context_t<legion_execution_policy_t> context_legion;

} // namespace flecsi

#endif // flecsi_context_legion_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
