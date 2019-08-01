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
#pragma once

/*!
  @file

  This file implements the basic task execution model interface.

  Attribution: The initial structure of this interface is due to Davis Herring,
  who deserves special recognition for an elegant improvement over our original
  macro interface.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/execution/common/launch.hh>
#include <flecsi/execution/common/task_attributes.hh>
#include <flecsi/runtime/execution_policy.hh>
#endif

namespace flecsi {

/*!
  Execute a reduction task.

  @tparam TASK                The user task.
  @tparam LAUNCH_DOMAIN       The launch domain.
  @tparam REDUCTION_OPERATION The reduction operation.
  @tparam ATTRIBUTES          The task attributes mask.
  @tparam ARGS                The user-specified task arguments.
 */

template<auto & TASK,
  size_t LAUNCH_DOMAIN,
  size_t REDUCTION_OPERATION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto) reduce(ARGS &&... args);

/*!
  Execute a task.

  @tparam TASK          The user task.
  @tparam LAUNCH_DOMAIN The launch domain id.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS          The user-specified task arguments.
 */

template<auto & TASK,
  size_t LAUNCH_DOMAIN = flecsi::index,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
decltype(auto)
execute(ARGS &&... args) {
  return reduce<TASK, LAUNCH_DOMAIN, flecsi_internal_hash(0), ATTRIBUTES>(
    std::forward<ARGS>(args)...);
} // execute

} // namespace flecsi
