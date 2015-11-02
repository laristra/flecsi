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

  template<typename T, typename ... Args>
  static int32_t execute_task(T && task, Args && ... args) {

    // FIXME: place-holder example of static argument processing
    static_for_each(std::make_tuple(args ...), [&](auto arg) {
      std::cout << "test" << std::endl;
      });

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

  // FIXME We may need task registration

  template<typename T, typename ... Args>
  static int32_t execute_task(T && task, Args && ... args) {
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
