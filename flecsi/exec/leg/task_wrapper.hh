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

#include "flecsi/exec/leg/bind_accessors.hh"
#include "flecsi/exec/leg/future.hh"
#include "flecsi/exec/leg/unbind_accessors.hh"
#include "flecsi/exec/task_attributes.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/annotation.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/function_traits.hh"
#include "flecsi/util/serialize.hh"
#include <flecsi/flog.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <string>
#include <utility>

namespace flecsi {

inline log::devel_tag task_wrapper_tag("task_wrapper");

namespace data {
template<class, std::size_t, std::size_t>
struct ragged_accessor;

namespace detail {
template<class A>
struct convert_accessor {
  using type = A;
  using Base = typename type::base_type;
  static const Base & put(const type & a) {
    return a.get_base();
  }
  static type get(Base b) {
    return b;
  }
};
} // namespace detail
} // namespace data

// Send and receive only the reference_base portion:
template<data::layout L, class T, std::size_t Priv>
struct util::serial_convert<data::accessor<L, T, Priv>> {
  using type = data::accessor<L, T, Priv>;
  using Rep = std::size_t;
  static Rep put(const type & r) {
    return r.identifier();
  }
  static type get(const Rep & r) {
    return type(r);
  }
};
template<class T, std::size_t Priv>
struct util::serial_convert<data::accessor<data::single, T, Priv>>
  : data::detail::convert_accessor<data::accessor<data::single, T, Priv>> {};
template<class T, std::size_t P, std::size_t OP>
struct util::serial_convert<data::ragged_accessor<T, P, OP>>
  : data::detail::convert_accessor<data::ragged_accessor<T, P, OP>> {};
template<data::layout L, class T, std::size_t Priv>
struct util::serial<data::mutator<L, T, Priv>> {
  using type = data::mutator<L, T, Priv>;
  template<class P>
  static void put(P & p, const type & m) {
    serial_put(p, m.get_base());
  }
  static type get(const std::byte *& b) {
    return serial_get<typename type::base_type>(b);
  }
};
template<class T, std::size_t Priv>
struct util::serial<data::mutator<data::ragged, T, Priv>> {
  using type = data::mutator<data::ragged, T, Priv>;
  template<class P>
  static void put(P & p, const type & m) {
    serial_put(p, std::tie(m.get_base(), m.get_grow()));
  }
  static type get(const std::byte *& b) {
    const serial_cast r{b};
    return {r, r};
  }
};
template<class T, std::size_t Priv>
struct util::serial<data::topology_accessor<T, Priv>,
  std::enable_if_t<!util::memcpyable_v<data::topology_accessor<T, Priv>>>>
  : util::serial_value<data::topology_accessor<T, Priv>> {};

template<auto & F, class... AA>
struct util::serial<exec::partial<F, AA...>,
  std::enable_if_t<!util::memcpyable_v<exec::partial<F, AA...>>>> {
  using type = exec::partial<F, AA...>;
  using Rep = typename type::Base;
  template<class P>
  static void put(P & p, const type & t) {
    serial_put(p, static_cast<const Rep &>(t));
  }
  static type get(const std::byte *& b) {
    return serial_get<Rep>(b);
  }
};

template<class T>
struct util::serial<future<T>> : util::serial_value<future<T>> {};

namespace exec::leg {
using run::leg::task;

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
tuple_get(const Legion::Task & t) {
  struct Check {
    const std::byte *b, *e;
    Check(const Legion::Task & t)
      : b(static_cast<const std::byte *>(t.args)), e(b + t.arglen) {}
    ~Check() {
      flog_assert(b == e, "Bad Task::arglen");
    }
  } ch(t);
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
  {
    log::devel_guard guard(task_wrapper_tag);
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

// A trivial wrapper for nullary functions.
template<auto & F>
auto
verb(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *) {
  return F();
}

/*!
 The task_wrapper type provides execution
 functions for user and MPI tasks.

 \tparam F the user task
 \tparam P the target processor type

 @ingroup legion-execution
 */

template<auto & F, task_processor_type_t P>
struct task_wrapper {

  using Traits = util::function_traits<decltype(F)>;
  using RETURN = typename Traits::return_type;
  using param_tuple = typename Traits::arguments_type;

  static constexpr task_processor_type_t LegionProcessor = P;

  /*!
    Execution wrapper method for user tasks.
   */

  static RETURN execute(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context context,
    Legion::Runtime * runtime) {
    {
      log::devel_guard guard(task_wrapper_tag);
      flog_devel(info) << "In execute_user_task" << std::endl;
    }

    // Unpack task arguments
    auto task_args = detail::tuple_get<param_tuple>(*task);

    namespace ann = util::annotation;
    auto tname = util::symbol<F>();
    unbind_accessors ub(task_args);
    (ann::rguard<ann::execute_task_bind>(tname)),
      bind_accessors(runtime, context, regions, task->futures)(task_args);

    if constexpr(std::is_same_v<RETURN, void>) {
      (ann::rguard<ann::execute_task_user>(tname)),
        apply(F, std::forward<param_tuple>(task_args));
      ann::rguard<ann::execute_task_unbind> ann_guard(tname);
      ub();
    }
    else {
      RETURN result = (ann::rguard<ann::execute_task_user>(tname),
        apply(F, std::forward<param_tuple>(task_args)));
      ann::rguard<ann::execute_task_unbind> ann_guard(tname);
      ub();
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

  static RETURN execute(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context context,
    Legion::Runtime * runtime) {
    {
      log::devel_guard guard(task_wrapper_tag);
      flog_devel(info) << "In execute_mpi_task" << std::endl;
    }

    flog_assert(!task->arglen, "unexpected task arguments");
    auto & c = run::context::instance();
    const auto p = static_cast<param_tuple *>(c.mpi_params);

    namespace ann = util::annotation;
    auto tname = util::symbol<F>();
    unbind_accessors ub(*p);
    (ann::rguard<ann::execute_task_bind>(tname)),
      bind_accessors(runtime, context, regions, task->futures)(*p);

    // Set the MPI function and make the runtime active.
    if constexpr(std::is_void_v<RETURN>) {
      (ann::rguard<ann::execute_task_user>(tname)),
        c.mpi_call([&] { apply(F, std::move(*p)); });
      ann::rguard<ann::execute_task_unbind> ann_guard(tname);
      ub();
    }
    else {
      std::optional<RETURN> result;
      (ann::rguard<ann::execute_task_user>(tname)),
        c.mpi_call([&] { result.emplace(std::apply(F, std::move(*p))); });
      ann::rguard<ann::execute_task_unbind> ann_guard(tname);
      ub();
      return std::move(*result);
    }

  } // execute
}; // task_wrapper

} // namespace exec::leg
} // namespace flecsi
