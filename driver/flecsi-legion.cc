/*~-------------------------------------------------------------------------~~*
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
 *~-------------------------------------------------------------------------~~*/

#include <flecsi/execution/task.h>
#include <flecsi/execution/legion_execution_policy.h>
#include <flecsi/utils/common.h>

#include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)

/*----------------------------------------------------------------------------*
 * Serial execution harness.
 *
 * This harness uses the default execution policy, meaning that it basically
 * just logs task entry and exit and calls the task directly with no other
 * runtime actions.
 *----------------------------------------------------------------------------*/

int main(int argc, char ** argv) {
  return flecsi::execution_t<legion_execution_policy_t>::execute_task(driver,
    argc, argv);
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
