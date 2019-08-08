/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include "flecsi/runtime/context_policy.hh"
#include "flecsi/utils/function_traits.hh"
#include <flecsi/execution/common/task_attributes.hh>
#include <flecsi/execution/legion/enactment/bind_accessors.hh>
#include <flecsi/execution/legion/enactment/unbind_accessors.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/flog.hh>
#endif

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <string>

flog_register_tag(task_wrapper);

namespace flecsi {
namespace execution {
namespace legion {

namespace detail {
/*!
  Register a task with Legion.

  @tparam RETURN The return type of the task.
  @tparam TASK   The legion task.
  \tparam A task attributes

  @ingroup legion-execution
 */

template<typename RETURN,
  RETURN (*TASK)(const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *),
  std::size_t A>
void register_task();
} // namespace detail

/*!
  Arbitrary index for each task.

  @tparam F          Legion task function.
  @tparam ATTRIBUTES A size_t holding the mask of the task attributes mask
                     \ref task_attributes_mask_t.
 */

template<auto & F, size_t A = flecsi::loc | flecsi::leaf>
inline const size_t task_id = context_t::instance().register_task(
  utils::symbol<F>(),
  detail::register_task<
    typename utils::function_traits<decltype(F)>::return_type,
    F,
    A>);

template<typename RETURN,
  RETURN (*TASK)(const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *),
  std::size_t A>
void
detail::register_task() {
  constexpr auto processor_type = mask_to_processor_type(A);
  static_assert(processor_type != mpi, "Legion tasks cannot use MPI");

  const std::string name = utils::symbol<*TASK>();
  {
    flog_tag_guard(task_wrapper);
    flog_devel(info) << "registering pure Legion task " << name << std::endl;
  }

  Legion::TaskVariantRegistrar registrar(task_id<*TASK, A>, name.c_str());
  Legion::Processor::Kind kind = processor_type == task_processor_type_t::toc
                                   ? Legion::Processor::TOC_PROC
                                   : Legion::Processor::LOC_PROC;
  registrar.add_constraint(Legion::ProcessorConstraint(kind));
  registrar.set_leaf(leaf_task(A));
  registrar.set_inner(inner_task(A));
  registrar.set_idempotent(idempotent_task(A));

  /*
    This section of conditionals is necessary because there is still
    a distinction between void and non-void task registration with
    Legion.
   */

  if constexpr(std::is_same_v<RETURN, void>) {
    Legion::Runtime::preregister_task_variant<TASK>(registrar, name.c_str());
  }
  else {
    Legion::Runtime::preregister_task_variant<RETURN, TASK>(
      registrar, name.c_str());
  } // if
} // registration_callback

/*!
 The task_wrapper type provides execution
 functions for user and MPI tasks.

 \tparam F the user task
 \tparam P the target processor type

 @ingroup legion-execution
 */

template<auto & F, task_processor_type_t P> // P is for specialization only
struct task_wrapper {

  using Traits = utils::function_traits<decltype(F)>;
  using RETURN = typename Traits::return_type;
  using ARG_TUPLE = typename Traits::arguments_type;

  static constexpr task_processor_type_t LegionProcessor = P;

  /*!
    Execution wrapper method for user tasks.
   */

  static RETURN execute(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context context,
    Legion::Runtime * runtime) {
    {
      flog_tag_guard(task_wrapper);
      flog_devel(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    ARG_TUPLE & task_args = *(reinterpret_cast<ARG_TUPLE *>(task->args));

    bind_accessors_t bind_accessors(runtime, context, regions, task->futures);
    bind_accessors.walk(task_args);

    if constexpr(std::is_same_v<RETURN, void>) {
      apply(F, std::forward<ARG_TUPLE>(task_args));

      // FIXME: Refactor
      // finalize_handles_t finalize_handles;
      // finalize_handles.walk(task_args);
    }
    else {
      RETURN result = apply(F, std::forward<ARG_TUPLE>(task_args));

      // FIXME: Refactor
      // finalize_handles_t finalize_handles;
      // finalize_handles.walk(task_args);

      return result;
    } // if
  } // execute_user_task

}; // struct task_wrapper

template<auto & F>
struct task_wrapper<F, mpi> {
  using Traits = utils::function_traits<decltype(F)>;
  using RETURN = typename Traits::return_type;
  using ARG_TUPLE = typename Traits::arguments_type;

  static constexpr task_processor_type_t LegionProcessor = loc;

  static void execute(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context context,
    Legion::Runtime * runtime) {
    // FIXME: Refactor
    //    {
    //      flog_tag_guard(task_wrapper);
    //      flog_devel(info) << "In execute_mpi_task" << std::endl;
    //    }

    // Unpack task arguments.
    ARG_TUPLE & mpi_task_args = *(reinterpret_cast<ARG_TUPLE *>(task->args));

    // FIXME: Refactor
    // init_handles_t init_handles(runtime, context, regions, task->futures);
    // init_handles.walk(mpi_task_args);

    // Set the MPI function and make the runtime active.
    context_t::instance().set_mpi_task([=] { apply(F, mpi_task_args); });
    context_t::instance().set_mpi_state(true);

    // FIXME: Refactor
    // finalize_handles_t finalize_handles;
    // finalize_handles.walk(mpi_task_args);
  }
};

} // namespace legion
} // namespace execution
} // namespace flecsi
