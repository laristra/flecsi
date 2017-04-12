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
/// \param single A boolean indicating whether the task can be executed
///               with a single launch.
/// \param index A boolean indicating whether the task can be executed
///              with an index launch.
/// \param config_options (Variadic) A set of booleans indicating the task
///                       config options.
///
#define __flecsi_internal_register_legion_task(task, processor,                \
  single, index, ...)                                                          \
                                                                               \
  namespace flecsi {                                                           \
  namespace execution {                                                        \
  namespace legion {                                                           \
                                                                               \
  /* Create a callback wrapper type for each invocation of this macro */       \
  template<                                                                    \
    typename RETURN,                                                           \
    processor_type_t PROCESSOR,                                                \
    bool SINGLE,                                                               \
    bool INDEX,                                                                \
    bool LEAF=false,                                                           \
    bool INNER=false,                                                          \
    bool IDEMPOTENT=false                                                      \
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
      Legion::TaskConfigOptions config_options{ LEAF, INNER, IDEMPOTENT };     \
                                                                               \
      switch(PROCESSOR) {                                                      \
        case processor_type_t::loc:                                            \
          lr_runtime::register_legion_task<RETURN, task>(                      \
            tid, lr_proc::LOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
            config_options, EXPAND_AND_STRINGIFY(task));                       \
          break;                                                               \
        case processor_type_t::toc:                                            \
          lr_runtime::register_legion_task<RETURN, task>(                      \
            tid, lr_proc::TOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
            config_options, EXPAND_AND_STRINGIFY(task));                       \
          break;                                                               \
      } /* switch */                                                           \
    } /* register_task */                                                      \
                                                                               \
  }; /* struct */                                                              \
                                                                               \
  template<                                                                    \
    processor_type_t PROCESSOR,                                                \
    bool SINGLE,                                                               \
    bool INDEX,                                                                \
    bool LEAF,                                                                 \
    bool INNER,                                                                \
    bool IDEMPOTENT                                                            \
  >                                                                            \
  struct task ## _internal_task__<                                             \
    void,                                                                      \
    PROCESSOR,                                                                 \
    SINGLE,                                                                    \
    INDEX,                                                                     \
    LEAF,                                                                      \
    INNER,                                                                     \
    IDEMPOTENT                                                                 \
  >                                                                            \
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
      Legion::TaskConfigOptions config_options{ LEAF, INNER, IDEMPOTENT };     \
                                                                               \
      switch(PROCESSOR) {                                                      \
        case processor_type_t::loc:                                            \
          lr_runtime::register_legion_task<task>(                              \
            tid, lr_proc::LOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
            config_options, EXPAND_AND_STRINGIFY(task));                       \
          break;                                                               \
        case processor_type_t::toc:                                            \
          lr_runtime::register_legion_task<task>(                              \
            tid, lr_proc::TOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
            config_options, EXPAND_AND_STRINGIFY(task));                       \
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
inline bool task ## _call_register() {                                         \
  bool retval(false);                                                          \
                                                                               \
  /* Task return type (trt) */                                                 \
  using task ## _trt_t =                                                       \
    typename utils::function_traits__<decltype(task)>::return_type;            \
                                                                               \
  if(processor_loc(processor)) {                                               \
    using task_type_t =                                                        \
      flecsi::execution::legion::task ## _internal_task__<                     \
      task ## _trt_t, processor_type_t::loc, single, index, ##__VA_ARGS__>;    \
                                                                               \
    /* Task key for lookups */                                                 \
    auto key = task_hash_t::make_key(                                          \
      reinterpret_cast<uintptr_t>(&task), processor,                           \
      make_launch<single, index, ##__VA_ARGS__>());                            \
                                                                               \
    /* Register the task */                                                    \
    retval = context_t::instance().register_task(                              \
      key, processor_type_t::loc, task_type_t::register_task);                 \
                                                                               \
    if(!retval) { return false; }                                              \
  } /* if */                                                                   \
                                                                               \
  if(processor_toc(processor)) {                                               \
    using task_type_t =                                                        \
      flecsi::execution::legion::task ## _internal_task__<                     \
      task ## _trt_t, processor_type_t::toc, single, index, ##__VA_ARGS__>;    \
                                                                               \
    /* Task key for lookups */                                                 \
    auto key = task_hash_t::make_key(                                          \
      reinterpret_cast<uintptr_t>(&task), processor,                           \
      make_launch<single, index, ##__VA_ARGS__>());                            \
                                                                               \
    /* Register the task */                                                    \
    retval = context_t::instance().register_task(                              \
      key, processor_type_t::toc, task_type_t::register_task);                 \
  } /* if */                                                                   \
                                                                               \
  return retval;                                                               \
}                                                                              \
                                                                               \
static bool task ## _task_registered = task ## _call_register();

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
