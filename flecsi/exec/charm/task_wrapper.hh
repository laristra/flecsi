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
#endif

#include "flecsi/exec/charm/bind_accessors.hh"
#include "flecsi/exec/charm/future.hh"
#include "flecsi/exec/charm/unbind_accessors.hh"
#include "flecsi/exec/task_attributes.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/function_traits.hh"
#include "flecsi/util/serialize.hh"
#include "unbind_accessors.hh"
#include <flecsi/flog.hh>

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

#include <string>
#include <utility>

namespace flecsi {

inline log::devel_tag task_wrapper_tag("task_wrapper");

// Send and receive only the reference_base portion:
template<class T, std::size_t Priv>
struct util::serial_convert<data::accessor<data::dense, T, Priv>> {
  using type = data::accessor<data::dense, T, Priv>;
  using Rep = std::size_t;
  static Rep put(const type & r) {
    return r.identifier();
  }
  static type get(const Rep & r) {
    return type(r);
  }
};
template<class T, std::size_t Priv>
struct util::serial_convert<data::accessor<data::singular, T, Priv>> {
  using type = data::accessor<data::singular, T, Priv>;
  using Base = typename type::base_type;
  static const Base & put(const type & a) {
    return a.get_base();
  }
  static type get(Base b) {
    return b;
  }
};
// NB: topology_accessor is trivially copyable.

template<class T>
struct util::serial_convert<exec::flecsi_future<T>> {
  using type = exec::flecsi_future<T>;
  struct Rep {};
  static Rep put(const type &) {
    return {};
  }
  static type get(const Rep &) {
    return {};
  }
};

namespace exec::charm {
using run::charm::task;

namespace detail {
inline task_id_t last_task; // 0 is the top-level task
/*!
  Register a task with Legion.

  @tparam RETURN The return type of the task.
  @tparam TASK   The legion task.
  \tparam A task attributes

  @ingroup legion-execution
 */

template<typename RETURN, task<RETURN> * TASK, std::size_t A>
void register_task();

template<class T>
struct decay : std::decay<T> {};
template<class... TT>
struct decay<std::tuple<TT...>> {
  using type = std::tuple<std::decay_t<TT>...>;
};

template<class T>
auto
tuple_get(const std::vector<std::byte>& buf) {
  struct Check {
    const std::byte *b, *e;
    Check(const std::vector<std::byte>& buf)
      : b(buf.data()), e(b + buf.size()) {}
    ~Check() {
      flog_assert(b == e, "Bad vector<byte>::size()");
    }
  } ch(buf);
  return util::serial_get<typename decay<T>::type>(ch.b);
}
} // namespace detail

/*!
  Arbitrary index for each task.

  @tparam F          Legion task function.
  @tparam ATTRIBUTES A size_t holding the mask of the task attributes mask
                     \ref task_attributes_mask_t.
 */

template<auto & F, size_t A = loc | leaf>
// 'extern' works around GCC bug #90493
extern const task_id_t
  task_id = (run::context::instance().register_init(detail::register_task<
               typename util::function_traits<decltype(F)>::return_type,
               F,
               A>),
    ++detail::last_task);

template<typename RETURN, task<RETURN> * TASK, std::size_t A>
void
detail::register_task() {
  constexpr auto processor_type = mask_to_processor_type(A);
  static_assert(processor_type != task_processor_type_t::mpi,
    "Legion tasks cannot use MPI");

  const std::string name = util::symbol<*TASK>();
  std::cout << "Registering " << name << std::endl;
  {
    log::devel_guard guard(task_wrapper_tag);
    flog_devel(info) << "registering pure Legion task " << name << std::endl;
  }

  // TODO: At this point we would register some task information with the
  // Charm++ runtime

} // registration_callback

// A trivial wrapper for nullary functions.
// TODO: Need a charm++ replacement for this?
template<auto & F>
auto
verb(std::vector<std::byte>& buf) {
  return F();
}

/*!
 The task_wrapper type provides execution
 functions for user and MPI tasks.

 \tparam F the user task
 \tparam P the target processor type

 @ingroup legion-execution
 */

template<auto & F, task_processor_type_t P> // P is for specialization only
struct task_wrapper {

  using Traits = util::function_traits<decltype(F)>;
  using RETURN = typename Traits::return_type;
  using param_tuple = typename Traits::arguments_type;

  static constexpr task_processor_type_t LegionProcessor = P;

  /*!
    Execution wrapper method for user tasks.
   */

  static RETURN execute(std::vector<std::byte>& buf) {
    {
      log::devel_guard guard(task_wrapper_tag);
      flog_devel(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    // TODO: Can we deserialize directly into the user's parameters (i.e., do
    // without finalize_handles)?
    auto task_args = detail::tuple_get<param_tuple>(buf);

    bind_accessors_t bind_accessors(buf);
    bind_accessors.walk(task_args);

    if constexpr(std::is_same_v<RETURN, void>) {
      apply(F, std::forward<param_tuple>(task_args));

      // FIXME: Refactor
      // finalize_handles_t finalize_handles;
      // finalize_handles.walk(task_args);
    }
    else {
      RETURN result = apply(F, std::forward<param_tuple>(task_args));

      // FIXME: Refactor
      // finalize_handles_t finalize_handles;
      // finalize_handles.walk(task_args);

      return result;
    } // if
  } // execute_user_task

}; // struct task_wrapper

template<auto & F>
struct task_wrapper<F, task_processor_type_t::mpi> {
  using Traits = util::function_traits<decltype(F)>;
  using RETURN = typename Traits::return_type;
  using param_tuple = typename Traits::arguments_type;

  static constexpr auto LegionProcessor = task_processor_type_t::loc;

  static RETURN execute(std::vector<std::byte>& buf) {
    // FIXME: Refactor
    //    {
    //      log::devel_guard guard(task_wrapper_tag);
    //      flog_devel(info) << "In execute_mpi_task" << std::endl;
    //    }

    // Unpack task arguments.
    param_tuple * p;
    flog_assert(buf.size() == sizeof p, "Bad Task::arglen");
    std::memcpy(&p, buf.data(), sizeof p);
    auto & mpi_task_args = *p;

    // FIXME: Refactor
    // init_handles_t init_handles(runtime, context, regions, task->futures);
    // init_handles.walk(mpi_task_args);

    // TODO: Is more needed for synchronization with an "MPI" task?
    if constexpr(std::is_same_v<RETURN, void>) {
      apply(F, std::move(mpi_task_args));
    } else {
      return apply(F, std::move(mpi_task_args));
    }

    // FIXME: Refactor
    // finalize_handles_t finalize_handles;
    // finalize_handles.walk(mpi_task_args);
  }
};

} // namespace exec::charm
} // namespace flecsi
