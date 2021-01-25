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

#include <cstring>
#include <stdint.h>
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
 The task_collect_dependencies_t type can be called to walk the task args before
 the task has run. This allows to ensure task dependencies be added to the
 execution flow.

 @ingroup execution
 */

struct task_collect_dependencies_t
  : public flecsi::utils::tuple_walker_u<task_collect_dependencies_t> {

  /*!
   Construct a task_collect_dependencies_t instance.
   */

  task_collect_dependencies_t() = default;

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

    clog_assert(h.future != nullptr, "invalid future handle");
    if(h.future->valid()) {
      dependencies_.push_back(*h.future);
    }
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(global_accessor_u<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    clog_assert(h.future != nullptr, "invalid future handle");
    if(h.future->valid()) {
      dependencies_.push_back(*h.future);
    }
  } // handle

  template<typename T, size_t PERMISSIONS>
  void handle(color_accessor_u<T, PERMISSIONS> & a) {
    auto & h = a.handle;

    clog_assert(h.future != nullptr, "invalid future handle");
    if(h.future->valid()) {
      dependencies_.push_back(*h.future);
    }
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

    clog_assert(h.future != nullptr, "invalid future handle");
    if(h.future->valid()) {
      dependencies_.push_back(*h.future);
    }
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
  void handle(ragged_mutator<T> & m) {
    handle(m.handle);
  }

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  template<typename T, size_t PERMISSIONS>
  void handle(data_client_handle_u<T, PERMISSIONS> h) {

    clog_assert(h.future != nullptr, "invalid future handle");
    if(h.future->valid()) {
      dependencies_.push_back(*h.future);
    }
  }

  /*!
   Handle individual list items
   */
  template<typename T,
    std::size_t N,
    template<typename, std::size_t>
    typename Container>
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

  template<typename... Ts>
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
    The futures that represent the dependencies of the current task on its
    arguments
   */
  std::vector<hpx_future_u<void>> dependencies_;

}; // struct task_collect_dependencies_t

} // namespace execution
} // namespace flecsi
