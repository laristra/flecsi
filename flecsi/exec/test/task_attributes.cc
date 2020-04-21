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

#include "flecsi/util/ftest.hh"

#define __FLECSI_PRIVATE__
#include "flecsi/exec/task_attributes.hh"

using namespace flecsi;

log::devel_tag task_attributes_tag("task_attributes");

int
task_attributes(int, char **) {
  FTEST {
    {
      constexpr size_t mask = loc | leaf;

      static_assert(exec::leaf_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::loc);
    }

    {
      constexpr size_t mask = loc | inner;

      static_assert(exec::inner_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::loc);
    }

    {
      constexpr size_t mask = loc | idempotent;

      static_assert(exec::idempotent_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::loc);
    }

    {
      constexpr size_t mask = toc | leaf;

      static_assert(exec::leaf_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::toc);
    }

    {
      constexpr size_t mask = toc | inner;

      static_assert(exec::inner_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::toc);
    }

    {
      constexpr size_t mask = toc | idempotent;

      static_assert(exec::idempotent_task(mask));
      static_assert(
        exec::mask_to_processor_type(mask) == exec::task_processor_type_t::toc);
    }
  };
}

ftest_register_driver(task_attributes);
