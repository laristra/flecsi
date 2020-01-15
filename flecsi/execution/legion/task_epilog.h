/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <type_traits>
#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/data/common/data_reference.h>
#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/type_traits.h>

clog_register_tag(epilog);

namespace flecsi {
namespace execution {

/*!
 The task_epilog_t type can be called to walk the task args after the
 task has run. This allows synchronization dependencies to be added
 to the execution flow.

 @ingroup execution
 */

struct task_epilog_t : public flecsi::utils::tuple_walker_u<task_epilog_t> {

  /*!
   Construct a task_epilog_t instance.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  task_epilog_t(Legion::Runtime * runtime, Legion::Context & context)
    : runtime(runtime), context(context) {} // task_epilog_t

  /*!
   FIXME: Need description

   @tparam T                     The data type referenced by the handle.
   @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                 indices of the index partition.
   @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                 indices of the index partition.
   @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                 indices of the index partition.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(dense_accessor_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    auto & h = a.handle;

    if(!h.global && !h.color) {
      bool write_phase{
        (SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw)};

      if(write_phase && (*h.write_phase_started)) {

        {
          clog(trace) << " WRITE PHASE EPILOGUE" << std::endl;
        } // scope

        // As user
        // Phase READ
        *(h.write_phase_started) = false;
        // better to move copy here than in prolog
      } // if write phase

    } // if global and color

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

    bool write_phase{(SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw)};

    if(write_phase && (*h.write_phase_started)) {

      clog(trace) << " WRITE PHASE EPILOGUE" << std::endl;

      *(h.write_phase_started) = false;
    } // if write phase
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
    auto & h = m.handle;

    if((*h.write_phase_started)) {
      clog(trace) << " WRITE PHASE EPILOGUE" << std::endl;
      *(h.write_phase_started) = false;
    } // if write phase
  }

  template<typename T>
  void handle(sparse_mutator<T> & m) {
    handle(m.ragged);
  }

  template<typename T, size_t PERMISSIONS>
  typename std::enable_if_t<
    std::is_base_of<topology::mesh_topology_base_t, T>::value>
  handle(data_client_handle_u<T, PERMISSIONS> & h) {
    bool write_phase;
    write_phase = (PERMISSIONS == wo) || (PERMISSIONS == rw);

    if(write_phase) {

      for(size_t i = 0; i < h.num_handle_entities; ++i) {
        auto & ent = h.handle_entities[i];
        if(*ent.write_phase_started) {
          {
            clog(trace) << " DATA CLIENT WRITE PHASE EPILOGUE" << std::endl;
          } // scope

          // As user
          // Phase READ
          *(ent.write_phase_started) = false;
        }
      } // for

    } // if write phase
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
    for(auto & item : list)
      handle(item);
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
   This method is a no-op and is called when the task argument does not match
   one of the handle types above. For example, these could be simple scalars
   passed to the task.
   */

  template<typename T>
  static
    typename std::enable_if_t<!std::is_base_of<dense_accessor_base_t, T>::value>
    handle(T &) {} // handle

  Legion::Runtime * runtime;
  Legion::Context & context;

}; // struct task_epilog_t

} // namespace execution
} // namespace flecsi
