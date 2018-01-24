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

#include <string>

#include <cinchlog.h>
#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/finalize_handles.h>
#include <flecsi/execution/legion/init_handles.h>
#include <flecsi/execution/legion/registration_wrapper.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/tuple_function.h>
#include <flecsi/utils/tuple_type_converter.h>
#include <flecsi/utils/tuple_walker.h>

clog_register_tag(wrapper);

namespace flecsi {
namespace execution {

/*!
 Wrapper to handle returns from user task execution.

 @tparam RETURN    The return type of the user task.
 @tparam ARG_TUPLE A std::tuple of the user task arguments.
 @tparam DELEGATE  The delegate function that invokes the user task.

 @ingroup legion-execution
 */

template<typename RETURN, typename ARG_TUPLE, RETURN (*DELEGATE)(ARG_TUPLE)>
struct execution_wrapper__ {

  /*!
   Execute the delegate function, storing the return value.
   */

  void execute(ARG_TUPLE && args) {
    value_ = (*DELEGATE)(std::forward<ARG_TUPLE>(args));
  } // execute
  
  /*!
   Recover the return value. This is similar to a future, but with
   immediate execution. This function is suitable for returning from
   the task wrapper types below to handle void vs. non-void return types.
   */

  RETURN
  get() {
    return value_;
  } // get

private:
  RETURN value_;

}; // struct execution_wrapper__

/*!
 Wrapper to handle void returns from user task execution.

 @tparam ARG_TUPLE A std::tuple of the user task arguments.
 @tparam DELEGATE  The delegate function that invokes the user task.

 @ingroup legion-execution
 */

template<typename ARG_TUPLE, void (*DELEGATE)(ARG_TUPLE)>
struct execution_wrapper__<void, ARG_TUPLE, DELEGATE> {

  /*!
   Execute the delegate function. No value is stored for void.
   */

  void execute(ARG_TUPLE && args) {
    (*DELEGATE)(std::forward<ARG_TUPLE>(args));
  } // execute

  /*!
   This function is suitable for returning from the task wrapper
   types below to handle void return types.
   */

  void get() {}

}; // struct execution_wrapper__

/*!
 Pure Legion task wrapper.

 @tparam RETURN The return type of the task.
 @tparam TASK   The legion task.

 @ingroup legion-execution
 */

template<
    typename RETURN,
    RETURN (*TASK)(
        const Legion::Task *,
        const std::vector<Legion::PhysicalRegion> &,
        Legion::Context,
        Legion::Runtime *)>
struct pure_task_wrapper__ {
  
  /*!
    The task_id_t type is a unique identifier for Legion tasks.
   */

  using task_id_t = Legion::TaskID;

  /*!
   Registration callback function for pure Legion tasks.
  
   @param tid The task id to assign to the task.
   @param processor A valid Legion processor type.
   @param launch A \ref launch_t with the launch parameters.
   @param A std::string containing the task name.
   */

  static void registration_callback(
      task_id_t tid,
      processor_type_t processor,
      launch_t launch,
      std::string & task_name) {
    {
      clog_tag_guard(wrapper);
      clog(info) << "Executing PURE registration callback (" << task_name << ")"
                 << std::endl;
    }

    // Create configuration options using launch information provided
    // by the user.
    Legion::TaskConfigOptions config_options{
        launch_leaf(launch), launch_inner(launch), launch_idempotent(launch)};

    switch (processor) {
      case processor_type_t::loc: {
        clog_tag_guard(wrapper);
        clog(info) << "Registering PURE loc task: " << task_name << " "
                   << launch << std::endl
                   << std::endl;
      }
        registration_wrapper__<RETURN, TASK>::register_task(
            tid, Legion::Processor::LOC_PROC, config_options, task_name);
        break;
      case processor_type_t::toc: {
        clog_tag_guard(wrapper);
        clog(info) << "Registering PURE toc task: " << task_name << std::endl
                   << std::endl;
      }
        registration_wrapper__<RETURN, TASK>::register_task(
            tid, Legion::Processor::TOC_PROC, config_options, task_name);
        break;
      case processor_type_t::mpi:
        clog(fatal) << "MPI type passed to pure legion registration"
                    << std::endl;
        break;
    } // switch
  } // registration_callback

}; // struct pure_task_wrapper__

/*!
 The task_wrapper__ type provides registation callback and execution
 functions for user and MPI tasks.

 @tparam RETURN    The return type of the user task.
 @tparam ARG_TUPLE A std::tuple of the user task arguments.
 @tparam DELEGATE  The delegate function that invokes the user task.
 @tparam KEY       A hash key identifying the task.

 @ingroup legion-execution
 */

template<
    size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
struct task_wrapper__ {

  /*!
    The task_id_t type is a unique identifier for Legion tasks.
   */

  using task_id_t = Legion::TaskID;
  
  /*!
   Registration callback function for user tasks.
  
   @param tid    The task id to assign to the task.
   @param launch A \ref launch_t with the launch parameters.
   @param name   A std::string containing the task name.
   */

  static void registration_callback(
      task_id_t tid,
      processor_type_t processor,
      launch_t launch,
      std::string & name) {
    {
      clog_tag_guard(wrapper);
      clog(info) << "Executing registration callback for " << name << std::endl;
    }

    // Create configuration options using launch information provided
    // by the user.
    Legion::TaskConfigOptions config_options{
        launch_leaf(launch), launch_inner(launch), launch_idempotent(launch)};

    switch (processor) {
      case processor_type_t::loc: {
        clog_tag_guard(wrapper);
        clog(info) << "Registering loc task: " << name << std::endl
                   << std::endl;
      }
        registration_wrapper__<RETURN, execute_user_task>::register_task(
            tid, Legion::Processor::LOC_PROC, config_options, name);
        break;
      case processor_type_t::toc: {
        clog_tag_guard(wrapper);
        clog(info) << "Registering toc task: " << name << std::endl
                   << std::endl;
      }
        registration_wrapper__<RETURN, execute_user_task>::register_task(
            tid, Legion::Processor::TOC_PROC, config_options, name);
        break;
      case processor_type_t::mpi: {
        clog_tag_guard(wrapper);
        clog(info) << "Registering MPI task: " << name << std::endl
                   << std::endl;
      }
        registration_wrapper__<void, execute_mpi_task>::register_task(
            tid, Legion::Processor::LOC_PROC, config_options, name);
        break;
    } // switch
  } // registration_callback

  /*!
    Execution wrapper method for user tasks.
   */

  static RETURN execute_user_task(
      const Legion::Task * task,
      const std::vector<Legion::PhysicalRegion> & regions,
      Legion::Context context,
      Legion::Runtime * runtime) {
    {
      clog_tag_guard(wrapper);
      clog(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    ARG_TUPLE & task_args = *(reinterpret_cast<ARG_TUPLE *>(task->args));

    init_handles_t init_handles(runtime, context, regions);
    init_handles.walk(task_args);

    // Execute the user's task
    // return (*DELEGATE)(task_args);
    execution_wrapper__<RETURN, ARG_TUPLE, DELEGATE> wrapper;
    wrapper.execute(std::forward<ARG_TUPLE>(task_args));

    finalize_handles_t finalize_handles;
    finalize_handles.walk(task_args);

    return wrapper.get();
  } // execute_user_task

  /*!
    Execution wrapper method for MPI tasks.
   */

  static void execute_mpi_task(
      const Legion::Task * task,
      const std::vector<Legion::PhysicalRegion> & regions,
      Legion::Context context,
      Legion::Runtime * runtime) {
    {
      clog_tag_guard(wrapper);
      clog(info) << "In execute_mpi_task" << std::endl;
    }

    // Unpack task arguments.
    ARG_TUPLE & mpi_task_args = *(reinterpret_cast<ARG_TUPLE *>(task->args));

    init_handles_t init_handles(runtime, context, regions);
    init_handles.walk(mpi_task_args);

    // Create bound function to pass to MPI runtime.
    std::function<void()> bound_mpi_task = std::bind(DELEGATE, mpi_task_args);

    // Set the MPI function and make the runtime active.
    context_t::instance().set_mpi_task(bound_mpi_task);
    context_t::instance().set_mpi_state(true);

    finalize_handles_t finalize_handles;
    finalize_handles.walk(mpi_task_args);

  } // execute_mpi_task

}; // struct task_wrapper__

} // namespace execution
} // namespace flecsi
