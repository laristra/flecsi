/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_new_task_wrapper_h
#define flecsi_execution_legion_new_task_wrapper_h

///
/// \file
/// \date Initial file creation: Apr 12, 2017
///

#include <cinchlog.h>
#include <legion.h>
#include <string>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/utils/common.h"

clog_register_tag(wrapper);

namespace flecsi {
namespace execution {

///
/// This macro is used to avoid code duplication below.
///
#define __registration_callback(name, execute_method)                          \
  static void                                                                  \
  name(                                                                        \
    task_id_t tid,                                                             \
    processor_type_t processor,                                                \
    launch_t launch,                                                           \
    std::string & task_name                                                    \
  )                                                                            \
  {                                                                            \
    {                                                                          \
    clog_tag_guard(wrapper);                                                   \
    clog(info) << "Executing registration callback (" <<                       \
      EXPAND_AND_STRINGIFY(name) << std::endl;                                 \
    } /* scope */                                                              \
                                                                               \
    Legion::TaskConfigOptions config_options{ launch_leaf(launch),             \
      launch_inner(launch), launch_idempotent(launch) };                       \
                                                                               \
    switch(processor) {                                                        \
      case processor_type_t::loc:                                              \
        Legion::HighLevelRuntime::register_legion_task<execute_method>(        \
          tid, Legion::Processor::LOC_PROC, launch_single(launch),             \
          launch_index(launch), AUTO_GENERATE_ID, config_options,              \
          task_name.c_str());                                                  \
        break;                                                                 \
      case processor_type_t::toc:                                              \
        Legion::HighLevelRuntime::register_legion_task<execute_method>(        \
          tid, Legion::Processor::TOC_PROC, launch_single(launch),             \
          launch_index(launch), AUTO_GENERATE_ID, config_options,              \
          task_name.c_str());                                                  \
        break;                                                                 \
    } /* switch */                                                             \
  } /* __registration_callback */

///
/// \class new_task_wrapper__ new_task_wrapper.h
/// \brief new_task_wrapper__ provides...
///
template<
  typename R,
  typename A
>
struct new_task_wrapper__
{
  using task_id_t = Legion::TaskID;

  ///
  /// Registration callback function for user tasks.
  ///
  /// \param tid The task id to assign to the task.
  /// \param processor A valid Legion processor type.
  /// \param launch A \ref launch_t with the launch parameters.
  /// \param A std::string containing the task name.
  ///
  __registration_callback(user_registration_callback, execute_user_task);

  ///
  /// Wrapper method for user tasks.
  ///
  static void execute_user_task(
    const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime
  )
  {
  } // execute_user_task

  ///
  /// Registration callback function for pure Legion tasks.
  ///
  /// \param tid The task id to assign to the task.
  /// \param processor A valid Legion processor type.
  /// \param launch A \ref launch_t with the launch parameters.
  /// \param A std::string containing the task name.
  ///
  __registration_callback(legion_registration_callback, execute_legion_task);

  ///
  /// Wrapper method for pure Legion tasks.
  ///
  static void execute_legion_task(
    const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime
  )
  {
  } // execute_legion_task

}; // struct new_task_wrapper__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_new_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
