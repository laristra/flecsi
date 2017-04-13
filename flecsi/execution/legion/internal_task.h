/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_internal_task_h
#define flecsi_execution_legion_internal_task_h

#include <legion.h>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/execution/legion/task_args.h"
#include "flecsi/utils/common.h"

///
/// \file
/// \date Initial file creation: Mar 31, 2017
///

// FIXME: Change template names to something readable
//        Add TaskConfigOptions

///
/// Convenience macro to create a task key from Legion task information.
///
/// \param task The Legion task to register.
/// \param processor The processor type \ref processor_t.
/// \param single A boolean indicating whether this task can be run as a
///               single task.
/// \param index A boolean indicating whether this task can be run as an
///              index space launch.
///
#define __flecsi_task_key(task, processor)                                     \
  task_hash_t::make_key(reinterpret_cast<uintptr_t>(&task), processor, 0UL)

///
/// This macro registers a internal Legion task.
///
/// \param task The Legion task to register.
/// \param processor A processor_mask_t specifying the supported processor
///                  types.
/// \param launch A launch_t specifying the launch options.
///
#define __flecsi_internal_register_legion_task(task, processor, launch)        \
                                                                               \
  /* Register the user task in the function table */                           \
  flecsi_register_function(task);                                              \
                                                                               \
  /* Register the user task with the execution policy */                       \
  bool task ## _task_registered =                                              \
    flecsi::execution::task_t::register_legion_task<                           \
      task ## _trt_t,                                                          \
      task ## _tat_t                                                           \
      >                                                                        \
    (reinterpret_cast<uintptr_t>(&task), processor, launch,                    \
    { EXPAND_AND_STRINGIFY(task) })

#define __flecsi_internal_task_args(name)                                      \
  std::make_pair(                                                              \
    sizeof(flecsi::execution::function_handle__<                               \
      typename flecsi::utils::function_traits__<decltype(name)>::return_type,  \
      typename flecsi::utils::function_traits__<decltype(name)>::arguments_type\
    >),                                                                        \
    flecsi::execution::function_handle__<                                      \
      typename flecsi::utils::function_traits__<decltype(name)>::return_type,  \
      typename flecsi::utils::function_traits__<decltype(name)>::arguments_type\
    >(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()))

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
