/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_task_h
#define flexi_task_h

#include <iostream>
#include <type_traits>

#include "context.h"
#include "../utils/static_for_each.h"

/*!
 * \file task.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flexi {

/*!
  \class execution_t task.h
  \brief execution_t provides...
*/

class default_execution_policy_t
{
protected:

  using return_type_t = int32_t;

  template<typename T, typename ... Args>
  static return_type_t execute_task(T && task, Args && ... args) {

#if 0
    // FIXME: place-holder example of static argument processing
    static_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });
#endif

    context_t::instance().entry();
    auto value = task(std::forward<Args>(args) ...);
    context_t::instance().exit();
    return value;
  } // execute
  
}; // class default_execution_policy_t

/*!
  \class execution_t task.h
  \brief execution_t provides...
*/

template<typename execution_policy_t = default_execution_policy_t>
class execution_t : public execution_policy_t
{
public:

  using return_type_t = typename execution_policy_t::return_type_t;

  // FIXME We may need task registration

  template<typename T, typename ... Args>
  static return_type_t execute_task(T && task, Args && ... args) {

    // Make sure that the task definition returns the correct type
    static_assert(std::is_same<return_type_t,
      decltype(task(std::forward<Args>(args)...))>(),
      "Tasks must return flexi::execution_t::return_type_t");

    // pass the task to the execution policy for handling
    return execution_policy_t::execute_task(std::forward<T>(task),
      std::forward<Args>(args) ...);
  } // execute_task

}; // class execution_t

} // namespace flexi

#define task

#endif // flexi_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
