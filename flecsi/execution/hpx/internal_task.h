/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Mar 31, 2017
//----------------------------------------------------------------------------//

#include <hpx/hpx.hpp>

#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>

#include <flecsi/utils/const_string.h>

//----------------------------------------------------------------------------//
//! @def flecsi_internal_task_key
//!
//! Convenience macro to create a task key from hpx task information.
//!
//! @param task      The hpx task to register.
//! @param processor The processor type \ref processor_t.
//! @param single    A boolean indicating whether this task can be run as a
//!                  single task.
//! @param index     A boolean indicating whether this task can be run as an
//!                  index space launch.
//!
//! @ingroup hpx-execution
//----------------------------------------------------------------------------//

#define flecsi_internal_task_key(task)                                       \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Use const_string_t interface to create the key */                         \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash()

//----------------------------------------------------------------------------//
//! @def flecsi_internal_register_hpx_task
//!
//! This macro registers an internal hpx task.
//!
//! @param task      The hpx task to register.
//! @param processor A processor_mask_t specifying the supported processor
//!                  types.
//! @param launch    A launch_t specifying the launch options.
//!
//! @ingroup hpx-execution
//----------------------------------------------------------------------------//

#define flecsi_internal_register_hpx_task(task, processor, launch)           \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy to register the task */                         \
  inline bool task ## _task_registered =                                       \
    flecsi::execution::hpx_execution_policy_t::register_hpx_task<              \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash(),        \
      typename flecsi::utils::function_traits_u<decltype(task)>::return_type,  \
      task                                                                     \
    >                                                                          \
    (processor, launch, { EXPAND_AND_STRINGIFY(task) })
