/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_internal_task_h
#define flecsi_execution_legion_internal_task_h

#include <legion.h>

#include "flecsi.h"

#if !defined(ENABLE_BOOST_PREPROCESSOR)
  #error ENABLE_BOOST_PREPROCESSOR not defined! \
    This file depends on Boost.Preprocessor!
#endif

#include <boost/preprocessor.hpp>

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
#define __flecsi_internal_make_legion_task_key(task, processor,                \
  single, index, ...)                                                          \
  task_hash_t::make_key(reinterpret_cast<uintptr_t>(&task),                    \
    processor, make_launch<single, index, ##__VA_ARGS__>());

///
/// This macro registers a internal Legion task.
///
/// \param task The Legion task to register.
/// \param processor The processor type \ref processor_t.
/// \param single A boolean indicating whether this task can be run as a
///               single task.
/// \param index A boolean indicating whether this task can be run as an
///              index space launch.
/// \param config_options A scoped expression containting the task
///                       configuration options.
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
    bool SINGLE=true,                                                          \
    bool INDEX=false,                                                          \
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
        case loc:                                                              \
          lr_runtime::register_legion_task<RETURN, task>(                      \
            tid, lr_proc::LOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
            config_options, EXPAND_AND_STRINGIFY(task));                       \
          break;                                                               \
        case toc:                                                              \
          lr_runtime::register_legion_task<RETURN, task>(                      \
            tid, lr_proc::LOC_PROC, SINGLE, INDEX, AUTO_GENERATE_ID,           \
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
  /* Task return type (trt) */                                                 \
  using task ## _trt_t =                                                       \
    typename utils::function_traits__<decltype(task)>::return_type;            \
                                                                               \
  /* Task type for registration method */                                      \
  using task ## _internal_task_t =                                             \
    flecsi::execution::legion::task ## _internal_task__<                       \
    task ## _trt_t, processor, single, index, ##__VA_ARGS__>;                  \
                                                                               \
  /* Task key for lookups */                                                   \
  auto task ## _task_key = task_hash_t::make_key(                              \
    reinterpret_cast<uintptr_t>(&task), processor,                             \
    make_launch<single, index, ##__VA_ARGS__>());                              \
                                                                               \
  /* Register the task */                                                      \
  bool task ## _task_registered = context_t::instance().register_task(         \
    task ## _task_key, task ## _internal_task_t::register_task);

#endif // flecsi_execution_legion_internal_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
