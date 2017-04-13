/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_wrapper_h
#define flecsi_execution_legion_task_wrapper_h

///
/// \file
/// \date Initial file creation: Apr 12, 2017
///

#include <cinchlog.h>
#include <legion.h>
#include <string>

#include "flecsi/data/accessor.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/legion/task_args.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/tuple_type_converter.h"
#include "flecsi/utils/tuple_walker.h"

clog_register_tag(wrapper);

namespace flecsi {
namespace execution {

// This is called to walk the task args before the user task functions runs
// once we have the corresponding physical regions
struct handle_args_ : public utils::tuple_walker__<handle_args_>{
  handle_args_(
    Legion::Runtime* runtime,
    Legion::Context context,
    const std::vector<Legion::PhysicalRegion>& regions
  )
  : runtime(runtime),
  context(context),
  regions(regions){}

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  void handle(
    data_handle__<T, EP, SP, GP> & h
  )
  {

  }

  template<typename T>
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::value>
  handle(T&){}

  Legion::Runtime* runtime;
  Legion::Context context;
  const std::vector<Legion::PhysicalRegion>& regions;
}; // struct handle_args_

template<
	typename RETURN,
	RETURN (*METHOD)(
		const Legion::Task *,
		const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
	)
>
struct legion_registration_wrapper__
{
	template<typename ... ARGS>
	static void register_task(ARGS && ... args) {
  	Legion::HighLevelRuntime::register_legion_task<RETURN, METHOD>(
			std::forward<ARGS>(args) ...);
	} // register_task
}; // struct legion_registration_wrapper__

template<
	void (*METHOD)(
		const Legion::Task *,
		const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
	)
>
struct legion_registration_wrapper__<void, METHOD>
{
	template<typename ... ARGS>
  static void register_task(ARGS && ... args) {
  	Legion::HighLevelRuntime::register_legion_task<METHOD>(
			std::forward<ARGS>(args) ...);
	} // register_task
}; // struct legion_registration_wrapper__

#if 0
template<typename RETURN, legion_task_signature_t<void>, typename ... ARGS>
struct legion_registration_wrapper2__
{
	static void register_task(ARGS && ... args) {
	} // register_task
}; // struct legion_registration_wrapper__
#endif

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
      task_name << ")" << std::endl;                                           \
    } /* scope */                                                              \
                                                                               \
    Legion::TaskConfigOptions config_options{ launch_leaf(launch),             \
      launch_inner(launch), launch_idempotent(launch) };                       \
                                                                               \
    switch(processor) {                                                        \
      case processor_type_t::loc:                                              \
				legion_registration_wrapper__<R, execute_method>::register_task(       \
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
      case processor_type_t::mpi:                                              \
        Legion::HighLevelRuntime::register_legion_task<execute_mpi_task>(      \
          tid, Legion::Processor::LOC_PROC, launch_single(launch),             \
          launch_index(launch), AUTO_GENERATE_ID, config_options,              \
          task_name.c_str());                                                  \
        break;                                                                 \
    } /* switch */                                                             \
  } /* __registration_callback */

///
/// \class task_wrapper__ task_wrapper.h
/// \brief task_wrapper__ provides...
///
template<
  typename R,
  typename A
>
struct task_wrapper__
{
  using user_task_args_t =
    typename utils::base_convert_tuple_type<
		accessor_base, data_handle__<void, 0, 0, 0>, A>::type;
  using task_args_t = legion_task_args__<R, A>;
  using user_task_handle_t = typename task_args_t::user_task_handle_t;
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
    {
    clog_tag_guard(wrapper);
    clog(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;

    // Push the Legion state
    context_t::instance().push_state(user_task_handle.key(),
      context, runtime, task, regions);

    handle_args_ handle_args(runtime, context, regions);
    handle_args.walk(task_args.user_args);

    user_task_handle(context_t::instance().function(user_task_handle.key()),
      task_args.user_args);

    // Pop the Legion state
    context_t::instance().pop_state(user_task_handle.key());
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
    {
    clog_tag_guard(wrapper);
    clog(info) << "In execute_legion_task" << std::endl;
    }

    // Unpack task arguments
    user_task_handle_t & user_task_handle =
			*(reinterpret_cast<user_task_handle_t *>(task->args));

    {
    clog_tag_guard(wrapper);
    clog(info) << "Task handle key " << user_task_handle.key() << std::endl;
    }

    user_task_handle(context_t::instance().function(user_task_handle.key()),
      std::make_tuple(task, regions, context, runtime));
  } // execute_legion_task

  ///
  /// Wrapper method for pure Legion tasks.
  ///
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

    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));
    user_task_handle_t & user_task_handle = task_args.user_task_handle;
    user_task_args_t & user_task_args = task_args.user_args;

    std::function<void()> bound_user_task =
			std::bind(*reinterpret_cast<std::function<void(user_task_args_t)> *>(
			context_t::instance().function(user_task_handle.key())), user_task_args);

			context_t::instance().set_mpi_user_task(bound_user_task);
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
