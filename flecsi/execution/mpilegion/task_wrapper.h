/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpilegion_task_wrapper_h
#define flecsi_execution_mpilegion_task_wrapper_h

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/tuple_filter.h"
#include "flecsi/utils/tuple_for_each.h"
#include "flecsi/utils/tuple_function.h"

#include "flecsi/data/data_handle.h"

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
  typename ... As
>
struct mpilegion_task_wrapper_
{
  //
  // Type definition for user task.
  //
  using user_task_t = std::function<R(As ...)>;

  using lr_runtime = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_proc = LegionRuntime::HighLevel::Processor;

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
  // This function is called by the context singleton to do the actual
  // registration of the task wrapper with the Legion runtime. The structure
  // of the logic used is really just an object factory pattern.
  ///
  static void runtime_registration(size_t tid)
  {
    switch(P) {
      case loc:
        lr_runtime::register_legion_task<execute>(tid, lr_proc::LOC_PROC, S, I);
        break;
      case toc:
        lr_runtime::register_legion_task<execute>(tid, lr_proc::TOC_PROC, S, I);
        break;
      case mpi:
        lr_runtime::register_legion_task<execute_mpi>(tid, lr_proc::LOC_PROC, S, I); 
    } // switch
  } // runtime_registration

  ///
  // This method executes the user's task after processing the arguments
  // from the Legion runtime.
  ///
  static R execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    // Set the context for this execution thread
    context_t::instance().set_state(context, runtime, task, regions);

    // Define a tuple type for the task arguments
    using task_args_t = std::tuple<user_task_t, As ...>;

    // Get the arguments that were passed to Legion on the task launch
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));

    // Get the user task
    user_task_t user_task = std::get<0>(task_args);

    // Get the user task arguments
    auto user_args = tuple_filter_index_<greater_than, task_args_t>(task_args);

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
  } // execute

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

    // Define a tuple type for the task arguments
     using task_args_t = std::tuple<user_task_t, As ...>;

    // Get the arguments that were passed to Legion on the task launch
     task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));

    // Get the user task
     user_task_t user_task = std::get<0>(task_args);

    // Get the user task arguments
    auto user_args = tuple_filter_index_<greater_than, task_args_t>(task_args);
 
          
     std::function<void()> shared_func_tmp =
        tuple_function_mpi(user_task, user_args);

    // std::function<void()> shared_func_tmp = std::bind(user_task,
    //     std::forward<As>(args) ...);     

     ext_legion_handshake_t::instance().shared_func_=shared_func_tmp;

     ext_legion_handshake_t::instance().call_mpi_=true;
  }

}; // class mpilegion_task_wrapper_

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpilegion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
