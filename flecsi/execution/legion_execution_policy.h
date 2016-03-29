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


#include "flecsi/execution/context_legion.h"
#include "flecsi/utils/tuple_for_each.h"


/*!
 * \file legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */
namespace flecsi
{

extern void top_level_task(int argc, char** argv);


/*!
  \class legion_execution_policy legion_execution_policy.h
  \brief legion_execution_policy provides...
 */
class legion_execution_policy_t
{

 public:
	  template <typename T>
	  static void driver_top_task(const LegionRuntime::HighLevel::Task *task,
	                             const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
	                             LegionRuntime::HighLevel::Context ctx, LegionRuntime::HighLevel::HighLevelRuntime *runtime)
	  {
		  using namespace LegionRuntime::HighLevel;
		  const InputArgs &args = HighLevelRuntime::get_input_args();


		  top_level_task(args.argc,args.argv);

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
	  using namespace LegionRuntime::HighLevel;
	  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
	  HighLevelRuntime::register_legion_task<legion_execution_policy_t::driver_top_task<T>>(TOP_LEVEL_TASK_ID,
	      Processor::LOC_PROC, true/*single*/, false/*index*/);



	  return HighLevelRuntime::start(argc, argv);

  } // execute_driver

  template <typename T, typename... Args>
  static return_type_t execute_task(T && task, Args &&... args)
  {
    utils::tuple_for_each(std::make_tuple(args...),
        [&](auto arg) { std::cout << "test" << std::endl; });

//    context_t<legion_execution_policy_t>::instance().entry();
    auto value = task(std::forward<Args>(args)...);
//    context_t<legion_execution_policy_t>::instance().exit();
    return value;
  } // execute_task

}; // class legion_execution_policy_t

} // namespace flecsi

#endif // flecsi_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
