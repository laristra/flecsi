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
#include <flecsi/runtime/execution_policy.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/function_traits.hh>
#endif

#include <iostream>
#include <string>

namespace flecsi {
namespace execution {

/*!
  Arbitrary index for each task. The parameters for the variable template
  should be consistent with the options requried to register a task with the
  backend runtime, i.e., the task function, the task type (leaf, inner,
  idempotent), and the processor type (loc, toc, mpi). These options may evolve
  over time, although this will likely be reflected in updates to the
  task_attributes_t and task_attributes_mask_t type, and will not affect this
  interface.

  @tparam TASK       The user task function.
  @tparam ATTRIBUTES A size_t holding the mask of the task attributes mask
                     \ref task_attributes_mask_t.
 */

template<auto & TASK, size_t ATTRIBUTES>
const inline size_t task_id;

/*!
  Execute a task.

  @tparam TASK          The user task.
  @tparam LAUNCH_DOMAIN The launch domain id.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS          The user-specified task arguments.
 */

template<auto & TASK,
  size_t LAUNCH_DOMAIN,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto)
execute(ARGS &&... args) {

  using traits_t = utils::function_traits_u<decltype(TASK)>;

  return execution_policy_t::template execute_task<LAUNCH_DOMAIN,
    ATTRIBUTES,
    flecsi_internal_hash(0),
    typename traits_t::return_type,
    typename traits_t::arguments_type>(
      task_id<TASK, ATTRIBUTES>,
      std::forward<ARGS>(args)...);
} // execute

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
decltype(auto)
reduce(ARGS &&... args) {

  using traits_t = utils::function_traits_u<decltype(TASK)>;

  return execution_policy_t::template execute_task<LAUNCH_DOMAIN,
    REDUCTION_OPERATION,
    typename traits_t::return_type,
    typename traits_t::arguments_type>(
    task_id<TASK, ATTRIBUTES>, std::forward<ARGS>(args)...);
} // execute

inline size_t next_task = 0;

namespace internal {

/*!
  Internal function to register tasks. IT IS UNNECESSARY FOR USERS TO INVOKE
  THIS FUNCTION!
 */

template<auto & TASK>
size_t
register_task(size_t attributes) {

  using traits_t = utils::function_traits_u<decltype(TASK)>;
  using args_t = typename traits_t::arguments_type;

  /*
    The 'plus' operator in the following lambda definition converts the lambda
    to a plain-old function pointer.
   */

  constexpr auto delegate = +[](args_t args) { return std::apply(TASK, args); };

  execution_policy_t::template register_task<typename traits_t::return_type,
    args_t,
    delegate>(next_task, attributes, utils::symbol<TASK>());

  return next_task++;
} // register_task

} // namespace internal

template<auto & TASK, size_t ATTRIBUTES>
const inline size_t task_id =
  internal::register_task<TASK>(ATTRIBUTES);

} // namespace execution
} // namespace flecsi
