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

#ifndef flecsi_new_legion_execution_policy_h
#define flecsi_new_legion_execution_policy_h

#include <cstdint>

#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_for_each.h"

#include "flecsi/execution/legion_context_policy.h"

/*!
 * \file new_legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

namespace flecsi
{

/*
 */
const size_t TOP_LEVEL_TASK_ID = 0;

/*!
  \class new_legion_execution_policy new_legion_execution_policy.h
  \brief new_legion_execution_policy provides...
 */
class new_legion_execution_policy_t
{
public:

  using context_t = context_<legion_context_policy_t>;

  static void runtime_driver(const LegionRuntime::HighLevel::Task *task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime)
    {
      context_t::instance().set_state(ctx, runtime, task, regions);

      const LegionRuntime::HighLevel::InputArgs & args =
        context_t::lr_runtime_t::get_input_args();

      std::cout << "In runtime_driver: " << args.argc << std::endl;
      driver(args.argc, args.argv);
    } // runtime_driver

  using return_type_t = int32_t;

  static int initialize(int argc, char ** argv)
  {
    context_t::lr_runtime_t::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    context_t::lr_runtime_t::register_legion_task<runtime_driver>(
      TOP_LEVEL_TASK_ID, context_t::lr_loc, true, false);

    return context_t::lr_runtime_t::start(argc, argv);
  } // initialize

  static int finalize() {
  }

#if 1
  template <typename T, typename... Args>
  static return_type_t execute_driver(T && task, Args &&... args)
  {

    return task(std::forward<Args>(args)...);
  } // execute_driver
#endif

  template<typename T, typename ... Args>
  void register_task(T && task)
  {
  } // register_task

  template<typename T, typename ... Args>
  static return_type_t execute_task(T && task, Args &&... args)
  {
#if 0
    // FIXME: place-holder example of static argument processing
    utils::tuple_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });
#endif

    context_t::instance().entry();
    auto value = task(std::forward<Args>(args)...);
    context_t::instance().exit();
    return value;
  } // execute_task

}; // class new_legion_execution_policy_t

} // namespace flecsi

#endif // flecsi_new_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
