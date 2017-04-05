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
#include "flecsi/utils/common.h"

///
/// \file
/// \date Initial file creation: Mar 31, 2017
///

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
#define __flecsi_internal_make_legion_task_key(task, processor, single, index) \
  task_hash_t::make_key(reinterpret_cast<uintptr_t>(&task),                    \
    processor, flecsi_bools_to_launch(single, index));

///
/// This macro registers a internal Legion task.
///
#define __flecsi_internal_register_legion_task(task, processor, single, index) \
                                                                               \
  namespace flecsi {                                                           \
  namespace execution {                                                        \
  namespace legion {                                                           \
                                                                               \
  /* Create a callback wrapper type for each invocation of this macro */       \
  template<                                                                    \
    processor_t P,                                                             \
    bool S,                                                                    \
    bool I,                                                                    \
    typename R                                                                 \
  >                                                                            \
  struct task ## _internal_task__                                              \
  {                                                                            \
    using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;             \
    using lr_proc = LegionRuntime::HighLevel::Processor;                       \
    using task_id_t = LegionRuntime::HighLevel::TaskID;                        \
                                                                               \
    /* Callback method to register a legion task from the runtime context. */  \
    static                                                                     \
    void                                                                       \
    register_task(                                                             \
      task_id_t tid                                                            \
    )                                                                          \
    {                                                                          \
      switch(P) {                                                              \
        case loc:                                                              \
          lr_runtime::register_legion_task<R, task>(                           \
            tid, lr_proc::LOC_PROC, S, I);                                     \
          break;                                                               \
        case toc:                                                              \
          lr_runtime::register_legion_task<R, task>(                           \
            tid, lr_proc::LOC_PROC, S, I);                                     \
          break;                                                               \
      } /* switch */                                                           \
    } /* register_task */                                                      \
                                                                               \
  }; /* struct */                                                              \
                                                                               \
  } /* namespace legion */                                                     \
  } /* namespace execution */                                                  \
  } /* namespace flecsi */                                                     \
                                                                               \
  /* Task return type (trt) */                                                 \
  using task ## _trt_t =                                                       \
    typename utils::function_traits__<decltype(task)>::return_type;            \
                                                                               \
  /* Task type for registration method */                                      \
  using task ## _internal_task_t =                                             \
    flecsi::execution::legion::task ## _internal_task__<                       \
    processor, single, index, task ## _trt_t>;                                 \
                                                                               \
  /* Task key for lookups */                                                   \
  auto task ## _task_key = task_hash_t::make_key(                              \
    reinterpret_cast<uintptr_t>(&task), processor,                             \
    flecsi_bools_to_launch(single, index));                                    \
                                                                               \
  /* Register the task */                                                      \
  bool task ## _task_registered = context_t::instance().register_task(         \
    task ## _task_key, task ## _internal_task_t::register_task);

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
