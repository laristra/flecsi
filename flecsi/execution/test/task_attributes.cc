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
#include <flecsi/execution/task_attributes.hh>

using namespace flecsi;

flog::devel_tag task_attributes_tag("task_attributes");

int
task_attributes(int, char **) {

  FTEST();

  {
    size_t mask = loc | leaf;

    ASSERT_TRUE(execution::leaf_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::loc);
  }

  {
    size_t mask = loc | inner;

    ASSERT_TRUE(execution::inner_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::loc);
  }

  {
    size_t mask = loc | idempotent;

    ASSERT_TRUE(execution::idempotent_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::loc);
  }

  {
    size_t mask = toc | leaf;

    ASSERT_TRUE(execution::leaf_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::toc);
  }

  {
    size_t mask = toc | inner;

    ASSERT_TRUE(execution::inner_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::toc);
  }

  {
    size_t mask = toc | idempotent;

    ASSERT_TRUE(execution::idempotent_task(mask));
    ASSERT_EQ(execution::mask_to_processor_type(mask),
      execution::task_processor_type_t::toc);
  }

  return FTEST_RESULT();
}

ftest_register_driver(task_attributes);
