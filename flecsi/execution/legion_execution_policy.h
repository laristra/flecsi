/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_execution_policy_h
#define flecsi_legion_execution_policy_h

#include <iostream>
#include <utility>
#include <legion.h>


#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_for_each.h"


/*!
 * \file legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */
namespace flecsi
{
class legion_execution_policy_t;

extern void top_level_task(context_<flecsi::legion_execution_policy_t> &&ctx,int argc, char** argv);

/*!
  \class legion_execution_policy legion_execution_policy.h
  \brief legion_execution_policy provides...
 */
class legion_execution_policy_t
{
 public: // Member Classes

	class context_ep
	{
	public:
		context_ep():task(NULL),regions( 
           std::vector<LegionRuntime::HighLevel::PhysicalRegion>()){}
		context_ep(const LegionRuntime::HighLevel::Task *_task,
           const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &_regions,
           LegionRuntime::HighLevel::Context ctx, 
           LegionRuntime::HighLevel::HighLevelRuntime *runtime) 
           : ctx_l(ctx),rt(runtime),task(_task),regions(_regions){}

	protected:
		 LegionRuntime::HighLevel::Context ctx_l;
		 LegionRuntime::HighLevel::HighLevelRuntime* rt;
		 const LegionRuntime::HighLevel::Task *task;
		 const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions;
	};

 public: // Member Functions
	  template <typename T>
	  static void driver_top_task(
       const LegionRuntime::HighLevel::Task *task,
	     const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
	     LegionRuntime::HighLevel::Context ctx, 
       LegionRuntime::HighLevel::HighLevelRuntime *runtime)
	     {
		     const LegionRuntime::HighLevel::InputArgs &args =
              LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();
		     top_level_task(context_<flecsi::legion_execution_policy_t>(0,
                     task,regions,ctx,runtime),args.argc,args.argv);

	     }

 protected:
  using return_type_t = int32_t;

  enum Task_IDs
  {
  	TOP_LEVEL_TASK_ID
  };




  template <typename T>
  static return_type_t execute_driver(T && task, int argc, char** argv)
  {
	  LegionRuntime::HighLevel::HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
	  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
           <legion_execution_policy_t::driver_top_task<T>>(TOP_LEVEL_TASK_ID,
	              LegionRuntime::HighLevel::Processor::LOC_PROC, true/*single*/, false/*index*/);

	  return LegionRuntime::HighLevel::HighLevelRuntime::start(argc, argv);

  } // execute_driver



  template <typename T, typename... Args>
  static return_type_t execute_task(T && task, Args &&... args)
  {
    utils::tuple_for_each(std::make_tuple(args...),
        [&](auto arg) { std::cout << "test" << std::endl; });

//    context_<legion_execution_policy_t>::instance().entry();
    auto value = task(std::forward<Args>(args)...);
//    context_<legion_execution_policy_t>::instance().exit();
    return value;
  } // execute_task


  // Builds up the function signature for a task from
  template<typename... sArgs,typename... aArgs,typename T>
  static void build_task_sig(context_<legion_execution_policy_t> && ctx,
							 std::tuple<sArgs...> && sArgT, std::tuple<aArgs...> && aArgT,
							 T && task)
  {

  }

}; // class legion_execution_policy_t

} // namespace flecsi

#endif // flecsi_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
