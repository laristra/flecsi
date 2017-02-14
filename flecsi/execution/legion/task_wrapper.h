/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_wrapper_h
#define flecsi_execution_legion_task_wrapper_h

#include "flecsi/data/data_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/task_args.h"

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
  #include "flecsi/execution/mpilegion/legion_handshake.h"
#endif

#include "flecsi/utils/common.h"

///
// \file legion/task_wrapper.h
// \authors bergen
// \date Initial file creation: Jul 24, 2016
///

namespace flecsi {
namespace execution {

///
// \brief
//
// \tparam P processor type
// \tparam S single type task
// \tparam I index type task
// \tparam R task return type
// \tparam As arguments
///
template<
  processor_t P,
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
  using user_task_args_t = typename task_args_t::user_task_args_t;

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

  template<typename T>
  using is_data_handle = std::is_base_of<data_handle_base,T>;

  ///
  // This function is called by the context singleton to do the actual
  // registration of the task wrapper with the Legion runtime. The structure
  // of the logic used is really just an object factory pattern.
  ///
  static
  void
  register_task(
    task_id_t tid
  )
  {
    switch(P) {
      case loc:
        lr_runtime::register_legion_task<R, execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case toc:
        lr_runtime::register_legion_task<R, execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
      case mpi:
        break;
    } // switch
  } // register_task

  ///
  // This method executes the user's task after processing the arguments
  // from the Legion runtime.
  ///
  static R execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    // Unpack task arguments
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;
    user_task_args_t & user_task_args = task_args.user_args;

    // Push the Legion state
    context_t::instance().push_state(user_task_handle.key,
      context, runtime, task, regions);

#if 0
    // FIXME: Working on processing data handles
    // Somehow (???) we are going to have to interleave the processed
    // data handle arguments back into the original slots...

    // Get the data handle task arguments
    auto data_args = tuple_filter_<is_data_handle, task_args_t>(task_args);
    std::cout << "data_args size: " <<
      std::tuple_size<decltype(data_args)>::value << std::endl;

    utils::tuple_for_each(data_args, [&](auto & element) {
      std::cout << "hello" << std::endl;
    });

    // Execute the user task
    return tuple_function(user_task, user_args);
#endif

    auto retval = user_task_handle(
      context_t::instance().function(user_task_handle.key),
      user_task_args);

    // Pop the Legion state
    context_t::instance().pop_state(user_task_handle.key);
    
    return retval;
  } // execute

#if 0
  static R execute_mpi(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
#ifdef LEGIONDEBUG
     int rank;
     MPI_Comm_rank( MPI_COMM_WORLD, &rank);
     std::cout<<"MPI rank from the index task = " << rank <<std::endl;
     ext_legion_handshake_t::instance().rank_=rank;
#endif

    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;
    user_task_args_t & user_task_args = task_args.user_args;

    // Get the user task arguments
//  auto user_args = tuple_filter_index_<greater_than, task_args_t>(task_args);
 
    auto bound_user_task = std::bind(reinterpret_cast<std::function<R(A)> *>(
      context_t::instance().function(user_task_handle.key)), user_task_args);

     ext_legion_handshake_t::instance().shared_func_ = bound_user_task;

     ext_legion_handshake_t::instance().call_mpi_=true;
  } // execute_mpi
#endif

}; // class legion_task_wrapper__

///
// Partial specialization for void.
///
template<
  processor_t P,
  bool S,
  bool I,
  typename A
>
struct legion_task_wrapper__<P, S, I, void, A>
{
  //
  // Type definition for user task.
  //
  using task_args_t = legion_task_args__<void,A>;
  using user_task_handle_t = typename task_args_t::user_task_handle_t;
  using user_task_args_t = typename task_args_t::user_task_args_t;

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

  template<typename T>
  using is_data_handle = std::is_base_of<data_handle_base, T>;

  ///
  // This function is called by the context singleton to do the actual
  // registration of the task wrapper with the Legion runtime. The structure
  // of the logic used is really just an object factory pattern.
  ///
  static
  void
  register_task(
    task_id_t tid
  )
  {
    switch(P) {
      case loc:
        lr_runtime::register_legion_task<execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case toc:
        lr_runtime::register_legion_task<execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
      case mpi:
        lr_runtime::register_legion_task<execute_mpi>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
#endif // FLECSI_RUNTIME_MODEL_mpilegion
    } // switch
  } // register_task

  ///
  // This method executes the user's task after processing the arguments
  // from the Legion runtime.
  ///
  static void execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    // Unpack task arguments
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;
    user_task_args_t & user_task_args = task_args.user_args;

    // Push the Legion state
    context_t::instance().push_state(user_task_handle.key,
      context, runtime, task, regions);

#if 0
    // FIXME: Working on processing data handles
    // Somehow (???) we are going to have to interleave the processed
    // data handle arguments back into the original slots...

    // Get the data handle task arguments
    auto data_args = tuple_filter_<is_data_handle, task_args_t>(task_args);
    std::cout << "data_args size: " <<
      std::tuple_size<decltype(data_args)>::value << std::endl;

    utils::tuple_for_each(data_args, [&](auto & element) {
      std::cout << "hello" << std::endl;
    });

    // Execute the user task
    return tuple_function(user_task, user_args);
#endif

    user_task_handle(context_t::instance().function(user_task_handle.key),
      user_task_args);

    // Pop the Legion state
    context_t::instance().pop_state(user_task_handle.key);
  } // execute

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
  static void execute_mpi(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
#ifdef LEGIONDEBUG
     int rank;
     MPI_Comm_rank( MPI_COMM_WORLD, &rank);
     std::cout<<"MPI rank from the index task = " << rank <<std::endl;
     ext_legion_handshake_t::instance().rank_=rank;
#endif

    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;
    user_task_args_t & user_task_args = task_args.user_args;

    // Get the user task arguments
//  auto user_args = tuple_filter_index_<greater_than, task_args_t>(task_args);
 
    std::function<void()> bound_user_task =
      std::bind(*reinterpret_cast<std::function<void(A)> *>(
      context_t::instance().function(user_task_handle.key)), user_task_args);

     ext_legion_handshake_t::instance().shared_func_ = bound_user_task;

     ext_legion_handshake_t::instance().call_mpi_=true;
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
