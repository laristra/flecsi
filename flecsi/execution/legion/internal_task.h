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
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/execution/legion/task_args.h"
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

#define __flecsi_internal_task_key(task, processor)                            \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call task_hash_t interface to create the key */                           \
  task_hash_t::make_key(reinterpret_cast<uintptr_t>(&task), processor, 0UL)

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_register_legion_task
//!
//! This macro registers a internal Legion task.
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
      typename flecsi::utils::function_traits__<decltype(task)>::return_type,  \
      task                                                                     \
    >                                                                          \
    (task_hash_t::make_key(                                                    \
      reinterpret_cast<uintptr_t>(&task), processor, launch),                  \
      { EXPAND_AND_STRINGIFY(task) })

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
