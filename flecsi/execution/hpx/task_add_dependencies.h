/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2020, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <hpx/include/lcos.hpp>

#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>

/*!
 @file
 @date Initial file creation: November 16, 2020
 */

#include <cstdint>
#include <string>
#include <vector>

#include "mpi.h"

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/data/data.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/context.h>

#include "flecsi/utils/mpi_type_traits.h"
#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/type_traits.h>

namespace flecsi {
namespace execution {

/*!
 The task_add_dependencies_t type can be called to walk the task args before
 the task has run. This allows to ensure task dependencies be added to the
 execution flow.

 @ingroup execution
 */

struct task_add_dependencies_t
  : public flecsi::utils::tuple_walker_u<task_add_dependencies_t> {

  /*!
   Construct a task_add_dependencies_t instance.
   */

  task_add_dependencies_t()
    : has_dependencies(false), promise(), future(promise.get_future()) {}

  /*!
   FIXME: Need a description.

   @tparam T                     The data type referenced by the handle.
   @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                 indices of the index partition.
   @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                 indices of the index partition.
   @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                 indices of the index partition.

   */

  template<typename Dense,
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(Dense & a,
    dense_accessor<T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS> &) {

    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }
    else {
      a.future = future;
      has_dependencies = true;
    }
  } // handle

  template<typename Global, typename T, size_t PERMISSIONS>
  void handle(Global & a, global_accessor_u<T, PERMISSIONS> &) {
    // Skip Read Only handles
    if constexpr(PERMISSIONS != ro) {
      a.future = future;
      has_dependencies = true;
    }
  } // handle

  template<typename Ragged,
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(Ragged & a,
    ragged_accessor<T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS> &) {

    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }
    else {
      a.future = future;
      has_dependencies = true;
    }
  } // handle

  template<typename Sparse,
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(Sparse & a1,
    sparse_accessor<T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS> & a2) {
    handle(a1, a2.ragged);
  } // handle

  template<typename Ragged, typename T2>
  void handle(Ragged & r1, ragged_mutator<T2> & r2) {
    r1.future = future;
    has_dependencies = true;
  }

  template<typename Sparse, typename T2>
  void handle(Sparse & m1, sparse_mutator<T2> & m2) {
    m1.future = future;
    has_dependencies = true;
  }

  template<typename Client, typename T, size_t PERMISSIONS>
  void handle(Client & h, data_client_handle_u<T, PERMISSIONS> &) {

    // Skip Read Only handles
    if constexpr(PERMISSIONS == ro) {
      return;
    }
    else {
      h.future = future;
      has_dependencies = true;
    }
  }

  /*!
    Handle individual list items
   */
  template<typename T1,
    typename T2,
    std::size_t N1,
    std::size_t N2,
    template<typename, std::size_t>
    typename Container1,
    template<typename, std::size_t>
    typename Container2>
  void handle(Container1<T1, N1> & list1, Container2<T2, N2> & list2) {

    static_assert(N1 == N2, "list sizes must match");
    auto it2 = list2.begin();
    for(auto it1 = list1.begin(); it1 != list1.end(); ++it1, ++it2) {
      handle(*it1, *it2);
    }
  }

  /*!
   * Handle tuple of items
   */

  template<typename... Ts1, typename... Ts2, size_t... I>
  void handle_tuple_items(std::tuple<Ts1...> & items1,
    std::tuple<Ts2...> & items2,
    std::index_sequence<I...>) {
    (handle(std::get<I>(items1), std::get<I>(items2)), ...);
  }

  template<typename... Ts1, typename... Ts2>
  void handle(std::tuple<Ts1...> & items1, std::tuple<Ts2...> & items2) {
    handle_tuple_items(
      items1, items2, std::make_index_sequence<sizeof...(Ts1)>{});
  }

  /*!
    This method is called on any task arguments that are not handles, e.g.
    scalars or those that did not need any special handling.
   */
  template<typename T1, typename T2>
  void handle(T1 &, T2 &) {} // handle

  /*!
    This future is used as a dependency for all arguments, if needed
   */
  bool has_dependencies;
  hpx::lcos::local::promise<void> promise;
  hpx_future_u<void> future;

}; // struct task_add_dependencies_t

} // namespace execution
} // namespace flecsi
