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

#if !defined(__FLECSI_PRIVATE__)
#error Do not inlcude this file directly!
#else
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/function_traits.h>
#include <flecsi/utils/macros.h>
#endif

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

/*!
  @def flecsi_internal_register_legion_task

  This macro registers an internal Legion task.

  @param task      The Legion task to register.
  @param processor A processor_mask_t specifying the supported processor
                  types.
  @param launch    A launch_t specifying the launch options.

  @ingroup legion-execution
*/

#define flecsi_internal_register_legion_task(task, processor, launch)          \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the execution policy to register the task */                         \
  inline bool task##_task_registered =                                         \
    flecsi::execution::legion_execution_policy_t::register_legion_task<        \
      flecsi_internal_hash(task),                                              \
      typename flecsi::utils::function_traits_u<decltype(task)>::return_type,  \
      task>(processor, launch, {flecsi_internal_stringify(task)})
