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

#include "context.h"

/*!
 * \file task.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flexi {

/*!
  \function execute
 */
template<typename F, typename ... Args>
int32_t execute(F && function, Args && ... args) {
  context_t::instance().entry();
  auto value = function(std::forward<Args>(args) ...);
  context_t::instance().exit();
  return value;
} // execute

} // namespace flexi

#endif // flexi_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
