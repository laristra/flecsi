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

#include "flecsi/exec/launch.hh"
#include "flecsi/exec/charm/future.hh"
#include "flecsi/exec/charm/task_prologue.hh"
#include "flecsi/exec/charm/task_wrapper.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/function_traits.hh"
#include <flecsi/flog.hh>

#include <functional>
#include <memory>
#include <type_traits>

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

namespace flecsi {

inline log::devel_tag execution_tag("execution");

namespace exec {
namespace detail {

// Remove const from under a reference, if there is one.
template<class T>
struct nonconst_ref {
  using type = T;
};

template<class T>
struct nonconst_ref<const T &> {
  using type = T &;
};

template<class T>
using nonconst_ref_t = typename nonconst_ref<T>::type;

// Serialize a tuple of converted arguments (or references to existing
// arguments where possible).  Note that is_constructible_v<const
// float&,const double&> is true, so we have to check
// is_constructible_v<float&,double&> instead.
template<class... PP, class... AA>
auto
serial_arguments(std::tuple<PP...> * /* to deduce PP */, AA &&... aa) {
  static_assert((std::is_const_v<std::remove_reference_t<const PP>> && ...),
    "Tasks cannot accept non-const references");
  return util::serial_put<std::tuple<std::conditional_t<
    std::is_constructible_v<nonconst_ref_t<PP> &, nonconst_ref_t<AA>>,
    const PP &,
    std::decay_t<PP>>...>>(
    {exec::replace_argument<PP>(std::forward<AA>(aa))...});
}

// Helper to deduce PP:
template<class... PP, class... AA>
void
mpi_arguments(std::optional<std::tuple<PP...>> & opt, AA &&... aa) {
  opt.emplace(exec::replace_argument<PP>(std::forward<AA>(aa))...);
}

template<class, class>
struct tuple_prepend;
template<class T, class... TT>
struct tuple_prepend<T, std::tuple<TT...>> {
  using type = std::tuple<T, TT...>;
};

} // namespace detail

template<auto & F, class Reduction, size_t Attributes, typename... Args>
auto
reduce_internal(Args &&... args) {
  using traits_t = util::function_traits<decltype(F)>;
  using return_t = typename traits_t::return_type;
  using param_tuple = typename traits_t::arguments_type;

  // This will guard the entire method
  log::devel_guard guard(execution_tag);

  // Get the FleCSI runtime context
  auto & flecsi_context = run::context::instance();

  // Get the processor type.
  constexpr auto processor_type = mask_to_processor_type(Attributes);

  const auto domain_size = [&args..., &flecsi_context] {
    if constexpr(processor_type == task_processor_type_t::mpi) {
      return launch_size<
        typename detail::tuple_prepend<launch_domain, param_tuple>::type>(
        launch_domain{flecsi_context.processes()}, args...);
    }
    else {
      (void)flecsi_context;
      return launch_size<param_tuple>(args...);
    }
  }();

  charm::task_prologue_t pro;
  pro.walk<param_tuple>(args...);

  std::optional<param_tuple> mpi_args;
  std::vector<std::byte> buf;
  if constexpr(processor_type == task_processor_type_t::mpi) {
    // MPI tasks must be invoked collectively from one task on each rank.
    // We therefore can transmit merely a pointer to a tuple of the arguments.
    // util::serial_put deliberately doesn't support this, so just memcpy it.
    detail::mpi_arguments(mpi_args, std::forward<Args>(args)...);
    const auto p = &*mpi_args;
    buf.resize(sizeof p);
    std::memcpy(buf.data(), &p, sizeof p);
  } else {
    buf = detail::serial_arguments(
      static_cast<param_tuple *>(nullptr), std::forward<Args>(args)...);
  }

  //------------------------------------------------------------------------//
  // Single launch
  //------------------------------------------------------------------------//

  using wrap = charm::task_wrapper<F, processor_type>;
  const auto task = charm::task_id<wrap::execute,
    (Attributes & ~mpi) | 1 << static_cast<std::size_t>(wrap::CharmProcessor)>;

  // TODO: Right now we just execute tasks inline which doesn't expose any
  // paralellism. Tasks should be converted to entry methods in charm or
  // something similar, ie charm tasks.
  if constexpr(std::is_same_v<decltype(domain_size), const std::monostate>) {
    future<return_t, launch_type_t::single> f;
    if constexpr(std::is_same_v<return_t, void>) {
      flecsi_context.execute<wrap>(buf);
    } else {
      f.return_ = flecsi_context.execute<wrap>(buf);
    }
    return f;
  } else {
    future<return_t, launch_type_t::index> f;
    if constexpr(std::is_same_v<return_t, void>) {
      flecsi_context.execute<wrap>(buf);
    } else {
      f.return_ = flecsi_context.execute<wrap>(buf);
    }
    return f;
  }
} // reduce_internal

} // namespace exec

template<auto & F, class Reduction, size_t Attributes, typename... Args>
auto
reduce(Args &&... args) {
  using namespace exec;

  // This will guard the entire method
  log::devel_guard guard(execution_tag);

  // Get the FleCSI runtime context
  auto & flecsi_context = run::context::instance();
  std::size_t & tasks_executed = flecsi_context.tasks_executed();
  ++tasks_executed;
#if defined(FLECSI_ENABLE_FLOG)
  /*if(tasks_executed % FLOG_SERIALIZATION_INTERVAL == 0 &&
     reduce_internal<detail::log_size, fold::max<std::size_t>, flecsi::mpi>()
         .get() > FLOG_SERIALIZATION_THRESHOLD)
    reduce_internal<log::send_to_one, void, flecsi::mpi>();*/
#endif

  return reduce_internal<F, Reduction, Attributes, Args...>(
    std::forward<Args>(args)...);
}

} // namespace flecsi
