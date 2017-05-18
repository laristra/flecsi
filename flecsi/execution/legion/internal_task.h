/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_internal_task_h
#define flecsi_execution_legion_internal_task_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Mar 31, 2017
//----------------------------------------------------------------------------//

#include <legion.h>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/utils/common.h"

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_task_key
//!
//! Convenience macro to create a task key from Legion task information.
//!
//! @param task      The Legion task to register.
//! @param processor The processor type \ref processor_t.
//! @param single    A boolean indicating whether this task can be run as a
//!                  single task.
//! @param index     A boolean indicating whether this task can be run as an
//!                  index space launch.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#define __flecsi_internal_task_key(task)                                       \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Use const_string_t interface to create the key */                         \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash()

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_register_legion_task
//!
//! This macro registers an internal Legion task.
//!
//! @param task      The Legion task to register.
//! @param processor A processor_mask_t specifying the supported processor
//!                  types.
//! @param launch    A launch_t specifying the launch options.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#define __flecsi_internal_register_legion_task(task, processor, launch)        \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy to register the task */                         \
  bool task ## _task_registered =                                              \
    flecsi::execution::legion_execution_policy_t::register_legion_task<        \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash(),        \
      typename flecsi::utils::function_traits__<decltype(task)>::return_type,  \
      task                                                                     \
    >                                                                          \
    (processor, launch, { EXPAND_AND_STRINGIFY(task) })

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
