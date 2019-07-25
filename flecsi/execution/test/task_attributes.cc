/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/common/task_attributes.hh>

flog_register_tag(task_attributes);

using namespace flecsi;

int
task_attributes(int argc, char ** argv) {

  FTEST();

  {
  size_t mask = loc | leaf;

  ASSERT_TRUE(execution::leaf_task(mask));
  } // scope

  return FTEST_RESULT();
}

ftest_register_driver(task_attributes);
