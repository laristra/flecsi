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
#error Do not include this file directly!
#else
#include <flecsi/execution/task.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/function_traits.h>
#endif

#define flecsi_internal_return_type(task)                                      \
  typename flecsi::utils::function_traits_u<decltype(task)>::return_type

#define flecsi_internal_arguments_type(task)                                   \
  typename flecsi::utils::function_traits_u<decltype(task)>::arguments_type

#define flecsi_internal_execute_task(task, launch_domain, operation, ...)       \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Execute the user task */                                                  \
  /* WARNING: This macro returns a future. Don't add terminations! */          \
  flecsi::execution::task_interface_t::execute_task<                           \
    flecsi_internal_hash(task),                                                \
    flecsi_internal_hash(operation),                                           \
    flecsi_internal_return_type(task),                                         \
    flecsi_internal_arguments_type(task)>(launch_domain, #__VA_ARGS__)
