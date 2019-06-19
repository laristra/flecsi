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

/*! @file */

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/execution/common/processor.hh>
#include <flecsi/execution/context.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/utils/const_string.hh>
#include <flecsi/utils/function_traits.hh>
#include <flecsi/utils/macros.hh>
#endif

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
  @param execution A task_execution_type_t specifying the task execution
                   options

  @ingroup legion-execution
*/

#define flecsi_internal_register_legion_task(task, processor, execution_type)  \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the execution policy to register the task */                         \
  inline bool task##_task_registered =                                         \
    flecsi::execution::legion_execution_policy_t::register_legion_task<        \
      flecsi_internal_hash(task),                                              \
      typename flecsi::utils::function_traits_u<decltype(task)>::return_type,  \
      task>(processor, execution_type, {flecsi_internal_stringify(task)})
