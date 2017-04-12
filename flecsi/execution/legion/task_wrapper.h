/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_wrapper_h
#define flecsi_execution_legion_task_wrapper_h

///
// \file legion/task_wrapper.h
// \authors bergen, nickm
// \date Initial file creation: Jul 24, 2016
///

#include <tuple>

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/task_args.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/tuple_type_converter.h"
#include "flecsi/utils/tuple_walker.h"

clog_register_tag(task_wrapper);

namespace flecsi {
namespace execution {

  // This is called to walk the task args before the user task functions runs
  // once we have the corresponding physical regions
  struct handle_args_ : public utils::tuple_walker__<handle_args_>{
    handle_args_(Legion::Runtime* runtime,
                 Legion::Context context,
                 const std::vector<Legion::PhysicalRegion>& regions)
    : runtime(runtime),
    context(context),
    regions(regions){}

    template<typename T, size_t EP, size_t SP, size_t GP>
    void handle(data_handle__<T, EP, SP, GP>& h){

    }

    template<typename T>
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::
      value>
    handle(T&){

    }

    Legion::Runtime* runtime;
    Legion::Context context;
    const std::vector<Legion::PhysicalRegion>& regions;
  };

///
/// \brief
///
/// \tparam P processor type
/// \tparam S single type task
/// \tparam I index type task
/// \tparam R task return type
/// \tparam As arguments
///
template<
  processor_type_t P,
  bool S,
  bool I,
  typename R,
  typename A
>
struct legion_task_wrapper__
{
  //
  // Type definition for user task.
  //
  using task_args_t = legion_task_args__<R,A>;
  using user_task_handle_t = typename task_args_t::user_task_handle_t;
  using args_t = A;

  // Legion type definitions
  using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_proc = LegionRuntime::HighLevel::Processor;
  using task_id_t = LegionRuntime::HighLevel::TaskID;

  using user_args_t = A;

  //
  // This defines a predicate function to pass to tuple_filter that
  // will select all tuple elements after the first index, i.e., 0.
  //
  template<typename T>
  using greater_than = std::conditional_t<(T()>0), std::true_type,
    std::false_type>;

  ///
  /// This function is called by the context singleton to do the actual
  /// registration of the task wrapper with the Legion runtime. The structure
  /// of the logic used is really just an object factory pattern.
  ///
  static
  void
  register_callback(
    task_id_t tid
  )
  {
    switch(P) {
      case processor_type_t::loc:
        lr_runtime::register_legion_task<R, execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case processor_type_t::toc:
        lr_runtime::register_legion_task<R, execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
      case processor_type_t::mpi:
        break;
    } // switch
  } // register_callback

  ///
  /// This method executes the user's task after processing the arguments
  /// from the Legion runtime.
  ///
  static R execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    {
    clog_tag_guard(task_wrapper);
    clog(info) << __PRETTY_FUNCTION__ << std::endl;
    }

    // Unpack task arguments
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;

    // Push the Legion state
    context_t::instance().push_state(user_task_handle.key(),
      context, runtime, task, regions);

    handle_args_
      handle_args(runtime, context, regions);
    handle_args.walk(task_args.user_args);

    auto retval = user_task_handle(
      context_t::instance().function(user_task_handle.key()),
      task_args.user_args);

    // Pop the Legion state
    context_t::instance().pop_state(user_task_handle.key());
    
    return retval;
  } // execute

}; // class legion_task_wrapper__

///
/// Partial specialization for void.
///
template<
  processor_type_t P,
  bool S,
  bool I,
  typename A
>
struct legion_task_wrapper__<P, S, I, void, A>
{
  //
  // Type definition for user task.
  //
  using args_t = A;

  using task_args_t = legion_task_args__<void, A>;
  using user_task_handle_t = typename task_args_t::user_task_handle_t;

  using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_proc = LegionRuntime::HighLevel::Processor;
  using task_id_t = LegionRuntime::HighLevel::TaskID;

  //
  // This defines a predicate function to pass to tuple_filter that
  // will select all tuple elements after the first index, i.e., 0.
  //
  template<typename T>
  using greater_than = std::conditional_t<(T()>0), std::true_type,
    std::false_type>;

  ///
  /// This function is called by the context singleton to do the actual
  /// registration of the task wrapper with the Legion runtime. The structure
  /// of the logic used is really just an object factory pattern.
  ///
  static
  void
  register_callback(
    task_id_t tid
  )
  {
    switch(P) {
      case processor_type_t::loc:
        lr_runtime::register_legion_task<execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case processor_type_t::toc:
        lr_runtime::register_legion_task<execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      case processor_type_t::mpi:
        lr_runtime::register_legion_task<execute_mpi>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
#endif // FLECSI_RUNTIME_MODEL_mpilegion
    } // switch
  } // register_callback

  ///
  /// This method executes the user's task after processing the arguments
  /// from the Legion runtime.
  ///
  static void execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    {
    clog_tag_guard(task_wrapper);
    clog(info) << __PRETTY_FUNCTION__ << std::endl;
    }

    // Unpack task arguments
    task_args_t & task_args = 
      *(reinterpret_cast<task_args_t *>(task->args));

    user_task_handle_t & user_task_handle = task_args.user_task_handle;

    // Push the Legion state
    context_t::instance().push_state(user_task_handle.key(),
      context, runtime, task, regions);

    handle_args_
      handle_args(runtime, context, regions);
    handle_args.walk(task_args.user_args);

    user_task_handle(context_t::instance().function(user_task_handle.key()),
      task_args.user_args);

    // Pop the Legion state
    context_t::instance().pop_state(user_task_handle.key());
  } // execute

  // FIXME: There is not a non-void version of this call. Is this what
  //        we want? This could be a valid restriction on the use of
  //        MPI tasks, but we should make it explicit in the docuemtation.
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion || \
  FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
  static void execute_mpi(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;

    // Convert the users task into something that we can execute.
    std::function<void()> bound_user_task =
      std::bind(*reinterpret_cast<std::function<void(task_args_t)> *>(
      context_t::instance().function(user_task_handle.key())), task_args);

     // Set the function for MPI to execute.
     context_t::instance().set_mpi_user_task(bound_user_task);

     // Set the call state to true so that the function will execute.
     context_t::instance().set_mpi_state(true);
  } // execute_mpi
#endif // FLECSI_RUNTIME_MODEL_mpilegion

}; // class legion_task_wrapper__

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
