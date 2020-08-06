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
#include "flecsi/exec/charm/reduction_wrapper.hh"
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
  return util::serial_put(std::tuple<std::conditional_t<
      std::is_constructible_v<nonconst_ref_t<PP> &, nonconst_ref_t<AA>>,
      const PP &,
      std::decay_t<PP>>...>(std::forward<AA>(aa)...));
}

} // namespace detail
} // namespace exec

template<auto & F,
  const exec::launch_domain & LAUNCH_DOMAIN,
  class REDUCTION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto)
reduce(ARGS &&... args) {
  using namespace exec;

  using traits_t = util::function_traits<decltype(F)>;
  using RETURN = typename traits_t::return_type;
  using param_tuple = typename traits_t::arguments_type;

  // This will guard the entire method
  log::devel_guard guard(execution_tag);

  // Get the FleCSI runtime context
  auto & flecsi_context = run::context::instance();

  // Get the processor type.
  constexpr auto processor_type = mask_to_processor_type(ATTRIBUTES);

  size_t domain_size = LAUNCH_DOMAIN.size();
  domain_size = domain_size == 0 ? flecsi_context.processes() : domain_size;

  ++flecsi_context.tasks_executed();

  charm::task_prologue_t pro(domain_size);
  pro.walk<param_tuple>(args...);

  std::optional<param_tuple> mpi_args;
  std::vector<std::byte> buf;
  if constexpr(processor_type == task_processor_type_t::mpi) {
    // MPI tasks must be invoked collectively from one task on each rank.
    // We therefore can transmit merely a pointer to a tuple of the arguments.
    // util::serial_put deliberately doesn't support this, so just memcpy it.
    mpi_args.emplace(std::forward<ARGS>(args)...);
    const auto p = &*mpi_args;
    buf.resize(sizeof p);
    std::memcpy(buf.data(), &p, sizeof p);
  } else {
    buf = detail::serial_arguments(
      static_cast<param_tuple *>(nullptr), std::forward<ARGS>(args)...);
  }

  //------------------------------------------------------------------------//
  // Single launch
  //------------------------------------------------------------------------//

  using wrap = charm::task_wrapper<F, processor_type>;
  const auto task = charm::task_id<wrap::execute,
    (ATTRIBUTES & ~mpi) | 1 << static_cast<std::size_t>(wrap::LegionProcessor)>;

  // TODO: Right now we just execute tasks inline which doesn't expose any
  // paralellism. Tasks should be converted to entry methods in charm or
  // something similar, ie charm tasks.
  if constexpr(LAUNCH_DOMAIN == single) {
    if constexpr(std::is_same_v<RETURN, void>) {
      flecsi_context.execute<wrap>(buf);
      return charm_future<RETURN, launch_type_t::single>();
    } else {
      return charm_future<RETURN, launch_type_t::single>(flecsi_context.execute<wrap>(buf));
    }
  } else {
    if constexpr(std::is_same_v<RETURN, void>) {
      flecsi_context.execute<wrap>(buf);
      return charm_future<RETURN, launch_type_t::index>();
    } else {
      return charm_future<RETURN, launch_type_t::index>(flecsi_context.execute<wrap>(buf));
    }
  }
} // execute_task

} // namespace flecsi
