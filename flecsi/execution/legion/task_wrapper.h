/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_wrapper_h
#define flecsi_execution_legion_task_wrapper_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 12, 2017
//----------------------------------------------------------------------------//

#include <cinchlog.h>
#include <legion.h>
#include <string>

#include "flecsi/data/accessor.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/legion/registration_wrapper.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/tuple_type_converter.h"
#include "flecsi/utils/tuple_walker.h"

clog_register_tag(wrapper);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The handle_args_ type provides a mechanism to walk the task args
//! before the user task function runs from within
//! task_wrapper__::execute_user_task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

struct handle_args_ : public utils::tuple_walker__<handle_args_>
{
  //--------------------------------------------------------------------------//
  //! Constructor.
  //!
  //! @param runtime_ The legion task runtime pointer.
  //! @param context_ The legion task context reference.
  //! @param regions_ The Legion physical regions.
  //--------------------------------------------------------------------------//

  handle_args_(
    Legion::Runtime * runtime_,
    Legion::Context & context_,
    const std::vector<Legion::PhysicalRegion> & regions_
  )
  :
    runtime(runtime_),
    context(context_),
    regions(regions_)
  {
  } // handle_args_

  //--------------------------------------------------------------------------//
  //! FIXME: Need description.
  //!
  //! @tparam T                     The data type referenced by the handle.
  //! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
  //!                               indices of the index partition.
  //! @tparam SHARED_PERMISSIONS    The permissions required on the shared
  //!                               indices of the index partition.
  //! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
  //!                               indices of the index partition.
  //!
  //! @param h The data handle.
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void handle(
    data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    > & h
  )
  {
  } // handle

  //--------------------------------------------------------------------------//
  //! FIXME: Need to document.
  //--------------------------------------------------------------------------//

  template<
    typename T
  >
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::value>
  handle(T&)
  {
  } // handle

  Legion::Runtime * runtime;
  Legion::Context & context;
  const std::vector<Legion::PhysicalRegion> & regions;

}; // struct handle_args_

//----------------------------------------------------------------------------//
// Pure Legion task registration.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! Pure Legion task wrapper.
//!
//! @tparam RETURN The return type of the task.
//! @tparam TASK   The legion task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  RETURN (*TASK)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct pure_task_wrapper__
{
  //--------------------------------------------------------------------------//
  //! The task_id_t type is a unique identifier for Legion tasks.
  //--------------------------------------------------------------------------//

  using task_id_t = Legion::TaskID;

  //--------------------------------------------------------------------------//
  //! Registration callback function for pure Legion tasks.
  //!
  //! @param tid The task id to assign to the task.
  //! @param processor A valid Legion processor type.
  //! @param launch A \ref launch_t with the launch parameters.
  //! @param A std::string containing the task name.
  //--------------------------------------------------------------------------//

  static
  void
  registration_callback(
    task_id_t tid,
    processor_type_t processor,
    launch_t launch,
    std::string & task_name
  )
  {
    {
    clog_tag_guard(wrapper);
    clog(info) << "Executing PURE registration callback (" <<
      task_name << ")" << std::endl;
    }

    // Create configuration options using launch information provided
    // by the user.
    Legion::TaskConfigOptions config_options{ launch_leaf(launch),
      launch_inner(launch), launch_idempotent(launch) };

    switch(processor) {
      case processor_type_t::loc:
        {
        clog_tag_guard(wrapper);
        clog(info) << "Registering PURE loc task: " <<
          task_name << std::endl << std::endl;
        }
        registration_wrapper__<RETURN, TASK>::register_task(
          tid, Legion::Processor::LOC_PROC, launch_single(launch),
          launch_index(launch), AUTO_GENERATE_ID, config_options,
          task_name.c_str());
        break;
      case processor_type_t::toc:
        {
        clog_tag_guard(wrapper);
        clog(info) << "Registering PURE loc task: " <<
          task_name << std::endl << std::endl;
        }
        registration_wrapper__<RETURN, TASK>::register_task(
          tid, Legion::Processor::TOC_PROC, launch_single(launch),
          launch_index(launch), AUTO_GENERATE_ID, config_options,
          task_name.c_str());
        break;
    } // switch
  } // registration_callback

}; // struct pure_task_wrapper__

//----------------------------------------------------------------------------//
// User and MPI task registration.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! The task_wrapper__ type provides registation and execution callback
//! functions for user and MPI tasks.
//!
//! @tparam RETURN    The return type of the user task.
//! @tparam ARG_TUPLE A std::tuple of the user task arguments.
//! @tparam DELEGATE  The delegate function that invokes the user task.
//! @tparam KEY       A hash key identifying the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  typename ARG_TUPLE,
  RETURN (*DELEGATE)(ARG_TUPLE),
  size_t KEY
>
struct task_wrapper__
{
  //--------------------------------------------------------------------------//
  //! The user_task_args_t type defines a task argument type for task
  //! execution through the Legion runtime.
  //--------------------------------------------------------------------------//

  using user_task_args_t =
    typename utils::base_convert_tuple_type<
    accessor_base, data_handle__<void, 0, 0, 0>, ARG_TUPLE>::type;

  //--------------------------------------------------------------------------//
  //! The task_id_t type is a unique identifier for Legion tasks.
  //--------------------------------------------------------------------------//

  using task_id_t = Legion::TaskID;

  //--------------------------------------------------------------------------//
  //! Registration callback function for user tasks.
  //!
  //! @param tid The task id to assign to the task.
  //! @param processor A valid Legion processor type.
  //! @param launch A \ref launch_t with the launch parameters.
  //! @param A std::string containing the task name.
  //--------------------------------------------------------------------------//

  static
  void
  registration_callback(
    task_id_t tid,
    processor_type_t processor,
    launch_t launch,
    std::string & task_name
  )
  {
    {
    clog_tag_guard(wrapper);
    clog(info) << "Executing registration callback (" <<
      task_name << ")" << std::endl;
    }

    // Create configuration options using launch information provided
    // by the user.
    Legion::TaskConfigOptions config_options{ launch_leaf(launch),
      launch_inner(launch), launch_idempotent(launch) };

    switch(processor) {
      case processor_type_t::loc:
        {
        clog_tag_guard(wrapper);
        clog(info) << "Registering loc task: " <<
          task_name << std::endl << std::endl;
        }
        registration_wrapper__<RETURN, execute_user_task>::register_task(
          tid, Legion::Processor::LOC_PROC, launch_single(launch),
          launch_index(launch), AUTO_GENERATE_ID, config_options,
          task_name.c_str());
        break;
      case processor_type_t::toc:
        {
        clog_tag_guard(wrapper);
        clog(info) << "Registering toc task: " <<
          task_name << std::endl << std::endl;
        }
        registration_wrapper__<RETURN, execute_user_task>::register_task(
          tid, Legion::Processor::TOC_PROC, launch_single(launch),
          launch_index(launch), AUTO_GENERATE_ID, config_options,
          task_name.c_str());
        break;
      case processor_type_t::mpi:
        {
        clog_tag_guard(wrapper);
        clog(info) << "Registering MPI task: " <<
          task_name << std::endl << std::endl;
        }
        registration_wrapper__<void, execute_mpi_task>::register_task(
          tid, Legion::Processor::LOC_PROC, launch_single(launch),
          launch_index(launch), AUTO_GENERATE_ID, config_options,
          task_name.c_str());
        break;
    } // switch
  } // registration_callback

  //--------------------------------------------------------------------------//
  //! Execution wrapper method for user tasks.
  //--------------------------------------------------------------------------//

  static RETURN execute_user_task(
    const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime
  )
  {
    {
    clog_tag_guard(wrapper);
    clog(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    user_task_args_t & user_task_args =
      *(reinterpret_cast<user_task_args_t *>(task->args));

    // Push the Legion state
    context_t::instance().push_state(KEY, context, runtime, task, regions);

    handle_args_ handle_args(runtime, context, regions);
    handle_args.walk(user_task_args);

    // FIXME: NEED TO HANDLE RETURN TYPES
    // Execute the user's task
    (*DELEGATE)(user_task_args);

    // Pop the Legion state
    context_t::instance().pop_state(KEY);

    // FIXME: NEED TO HANDLE RETURN TYPES
  } // execute_user_task

  //--------------------------------------------------------------------------//
  //! Execution wrapper method for MPI tasks.
  //--------------------------------------------------------------------------//

  static void execute_mpi_task(
    const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime
  )
  {
    {
    clog_tag_guard(wrapper);
    clog(info) << "In execute_mpi_task" << std::endl;
    }

    // Unpack task arguments.
    ARG_TUPLE & mpi_task_args = *(reinterpret_cast<ARG_TUPLE *>(task->args));

    // Create bound function to pass to MPI runtime.
    std::function<void()> bound_mpi_task = std::bind(DELEGATE, mpi_task_args);

    // Set the MPI function and make the runtime active.
    context_t::instance().set_mpi_task(bound_mpi_task);
    context_t::instance().set_mpi_state(true);
  } // execute_mpi_task

}; // struct task_wrapper__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
