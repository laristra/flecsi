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

#include <cstdint>

#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_for_each.h"
#include "flecsi/utils/const_string.h"

#include "flecsi/execution/legion/legion_context_policy.h"
#include "flecsi/execution/legion/legion_task_wrapper.h"

/*!
 * \file legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {

/*!
  \class legion_execution_policy legion_execution_policy.h
  \brief legion_execution_policy provides...
 */
class legion_execution_policy_t
{
public:

  using return_type_t = int32_t;
  using task_key_t = const_string_<uint16_t>;
  //using task_key_t = const_string_<uint32_t>;

#if 0
  template <typename T, typename... Args>
  static return_type_t execute_driver(T && task, Args &&... args)
  {
    return task(std::forward<Args>(args)...);
  } // execute_driver
#endif

  /*
    To add:
      processor type
      task type (leaf, inner, etc...)
   */
  template<typename R, typename ... Args>
  static bool register_task(const task_key_t & key) {
      using task_wrapper_t = legion_task_wrapper_<R, Args ...>;
      std::cout << "registering " << key.hash() << std::endl;
      return context_t::instance().register_task(key.hash(),
        task_wrapper_t::runtime_registration);
  } // register_task

  template<typename ... Args>
  static return_type_t execute_task(const task_key_t & key, Args &&... args)
  {
    using namespace Legion;

    std::cout << "trying to execute " << key.hash() << std::endl;

    context_t & context_ = context_t::instance();

    TaskLauncher task_launcher(key.hash(), TaskArgument(nullptr, 0));
    //TaskLauncher task_launcher(100000, TaskArgument(nullptr, 0));

    context_.runtime()->execute_task(context_.context(), task_launcher);

#if 0
    // FIXME: place-holder example of static argument processing
    utils::tuple_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });

    context_t::instance().entry();
    auto value = task(std::forward<Args>(args)...);
    context_t::instance().exit();
    return value;
#endif
  } // execute_task

#if 0
  kernel_handle_t register_kernel(const const_string_t & key, T && kernel,
    Args &&... args)
  {
  } // register_kernel
#endif

}; // class legion_execution_policy_t

} // namespace flecsi

#endif // flecsi_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
