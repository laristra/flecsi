/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_wrapper_h
#define flecsi_execution_legion_task_wrapper_h

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"
//#include "flecsi/utils/tuple_filter.h"
//#include "flecsi/utils/tuple_for_each.h"
//#include "flecsi/utils/tuple_function.h"

#include "flecsi/data/data_handle.h"

///
// \file legion/task_wrapper.h
// \authors bergen
// \date Initial file creation: Jul 24, 2016
///

namespace flecsi {
namespace execution {

///
// Argument type for wrapper task. This type is used to pass the
// user task and arguments from the call site to the Legion runtime.
///
template<
  typename R,
  typename A
>
struct legion_task_args__
{
  using user_task_t = std::function<R(A)>;
  using user_task_args_t = A;

  legion_task_args__(user_task_t & user_task_, user_task_args_t & user_args_)
    : user_task(user_task_), user_args(user_args_) {}

  user_task_t user_task;
  user_task_args_t user_args;
}; // struct legion_task_args__

///
// \brief
//
// \tparam P ...
// \tparam S ...
// \tparam I ...
// \tparam R ...
// \tparam As ...
///
template<
  processor_t P,
  bool S,
  bool I,
  typename R,
  typename A
>
struct legion_task_wrapper_
{
  //
  // Type definition for user task.
  //
  using task_args_t = legion_task_args__<R,A>;
  using user_task_t = typename task_args_t::user_task_t;
  using user_task_args_t = typename task_args_t::user_task_args_t;

  //
  // This defines a predicate function to pass to tuple_filter that
  // will select all tuple elements after the first index, i.e., 0.
  //
  template<typename T>
  using greater_than = std::conditional_t<(T()>0), std::true_type,
    std::false_type>;

  template<typename T>
  using is_data_handle = std::is_base_of<data_handle_t,T>;

  ///
  // This method executes the user's task after processing the arguments
  // from the Legion runtime.
  ///
  static R execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    // Set the context for this execution thread
    context_t::instance().set_state(context, runtime, task, regions);

    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_t & user_task = task_args.user_task;
    user_task_args_t & user_task_args = task_args.user_args;

    return user_task(user_task_args);
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
  } // execute

}; // class legion_task_wrapper_

template<
  processor_t P,
  bool S,
  bool I,
  typename R,
  typename A,
  bool is_void = std::is_void<R>::value
>
struct legion_task_registrar__
{
  using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_proc = LegionRuntime::HighLevel::Processor;
  using task_id_t = LegionRuntime::HighLevel::TaskID;

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
    using wrapper_t = legion_task_wrapper_<P, S, I, R, A>;

    switch(P) {
      case loc:
        lr_runtime::register_legion_task<wrapper_t::execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case toc:
        lr_runtime::register_legion_task<wrapper_t::execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
      case mpi:
        break;
    } // switch
  } // register_task
}; // legion_task_registrar__

template<
  processor_t P,
  bool S,
  bool I,
  typename R,
  typename A
>
struct legion_task_registrar__<P, S, I, R, A, false>
{
  using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_proc = LegionRuntime::HighLevel::Processor;
  using task_id_t = LegionRuntime::HighLevel::TaskID;

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
    using wrapper_t = legion_task_wrapper_<P, S, I, R, A>;

    switch(P) {
      case loc:
        lr_runtime::register_legion_task<R, wrapper_t::execute>(
          tid, lr_proc::LOC_PROC, S, I);
        break;
      case toc:
        lr_runtime::register_legion_task<R, wrapper_t::execute>(
          tid, lr_proc::TOC_PROC, S, I);
        break;
      case mpi:
        break;
    } // switch
  } // register_task
}; // legion_task_registrar__

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
