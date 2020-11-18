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

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {

    auto & h = a.handle;
    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }

    h.future = future;
    has_dependencies = true;
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    // Skip Read Only handles
    if(PERMISSIONS == ro)
      return;

    h.future = future;
    has_dependencies = true;
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    // Skip Read Only handles
    if constexpr((SHARED_PERMISSIONS == ro) || (GHOST_PERMISSIONS == rw) ||
                 (GHOST_PERMISSIONS == wo)) {
      return;
    }

    h.future = future;
    has_dependencies = true;
  } // handle

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(sparse_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    handle(a.ragged);
  } // handle

  template<typename T>
  void handle(ragged_mutator<T> & m) {} // handle

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  /*!
   Handle individual list items
   */
  template<typename T,
    std::size_t N,
    template<typename, std::size_t>
    typename Container,
    typename =
      std::enable_if_t<std::is_base_of<data::data_reference_base_t, T>::value>>
  void handle(Container<T, N> & list) {
    for(auto & item : list) {
      handle(item);
    }
  }

  /*!
   * Handle tuple of items
   */

  template<typename... Ts, size_t... I>
  void handle_tuple_items(std::tuple<Ts...> & items,
    std::index_sequence<I...>) {
    (handle(std::get<I>(items)), ...);
  }

  template<typename... Ts,
    typename = std::enable_if_t<
      utils::are_base_of_t<data::data_reference_base_t, Ts...>::value>>
  void handle(std::tuple<Ts...> & items) {
    handle_tuple_items(items, std::make_index_sequence<sizeof...(Ts)>{});
  }

  /*!
    This method is called on any task arguments that are not handles, e.g.
    scalars or those that did not need any special handling.
   */
  template<typename T>
  void handle(T &) {} // handle

  /*!
    This future is used as a dependency for all arguments, if needed
   */
  bool has_dependencies;
  hpx::lcos::local::promise<void> promise;
  hpx_future_u<void> future;

}; // struct task_add_dependencies_t

} // namespace execution
} // namespace flecsi
