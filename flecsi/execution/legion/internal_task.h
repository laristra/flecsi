/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>

#include <flecsi/utils/const_string.h>

/*!
  @def flecsi_internal_task_key

  Convenience macro to create a task key from Legion task information.

  @param task      The Legion task to register.
  @param processor The processor type \ref processor_t.
  @param single    A boolean indicating whether this task can be run as a
                   single task.
  @param index     A boolean indicating whether this task can be run as an
                   index space launch.

 @ingroup legion-execution
 */

#define flecsi_internal_task_key(task)                                       \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Use const_string_t interface to create the key */                         \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash()

/*!
  @def flecsi_internal_register_legion_task

  This macro registers an internal Legion task.

  @param task      The Legion task to register.
  @param processor A processor_mask_t specifying the supported processor
                  types.
  @param launch    A launch_t specifying the launch options.

  @ingroup legion-execution
*/

#define flecsi_internal_register_legion_task(task, processor, launch)        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the execution policy to register the task */                         \
  inline bool task##_task_registered =                                         \
      flecsi::execution::legion_execution_policy_t::register_legion_task<      \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash(),    \
          typename flecsi::utils::function_traits_u<decltype(                  \
              task)>::return_type,                                             \
          task>(processor, launch, {EXPAND_AND_STRINGIFY(task)})
