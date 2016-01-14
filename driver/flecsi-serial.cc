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

#include <flexi/execution/task.h>
#include <flexi/utils/common.h>

#include EXPAND_AND_STRINGIFY(FLEXI_DRIVER)

/*----------------------------------------------------------------------------*
 * Serial execution harness.
 *
 * This harness uses the default execution policy, meaning that it basically
 * just logs task entry and exit and calls the task directly with no other
 * runtime actions.
 *----------------------------------------------------------------------------*/

int main(int argc, char ** argv) {
  return flexi::execution_t<>::execute_task(driver, argc, argv);
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
