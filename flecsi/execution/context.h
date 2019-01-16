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

#include <flecsi/execution/global_object_wrapper.h>
#include <flecsi/utils/flog.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <unordered_map>

flog_register_tag(context);

namespace flecsi {
namespace execution {

/*!
  The context_u type provides a high-level execution context interface that
  is implemented by the given context policy.

  @tparam CONTEXT_POLICY The backend context policy.

  @ingroup execution
 */

template<class CONTEXT_POLICY>
struct context_u : public CONTEXT_POLICY {

  /*!
    Meyer's singleton instance.
   */

  static context_u & instance() {
    static context_u context;
    return context;
  } // instance

  /*--------------------------------------------------------------------------*
    Runtime interface.
   *--------------------------------------------------------------------------*/

  /*!
    Start the FleCSI runtime.

    @param argc The number of command-line arguments.
    @param argv The command-line arguments in a char **.

    @return An integer with \em 0 being success, and any other value
            being failure.
   */

  int start(int argc, char ** argv) {
    return CONTEXT_POLICY::start(argc, argv);
  } // start

  /*!
    Get the color of this process.
   */

  size_t color() const {
    return CONTEXT_POLICY::color();
  } // color

  /*!
    Get the number of colors.
   */

  size_t colors() const {
    return CONTEXT_POLICY::colors();
  } // color

  using top_level_action_t = std::function<int(int, char **)>;

  /*!
    Set the top-level action.
   */

  bool register_top_level_action(top_level_action_t tla) {
    top_level_action_ = tla;
    return true;
  } // register_top_level_action

  /*!
    Return the top-level action.
   */

  top_level_action_t & top_level_action() {
    return top_level_action_;
  } // top_level_action

  /*--------------------------------------------------------------------------*
    Reduction interface.
   *--------------------------------------------------------------------------*/

  bool register_reduction_operation(size_t key,
    const std::function<void()> & callback) {
    reduction_registry_[key] = callback;
    return true;
  } // register_reduction_operation

  std::map<size_t, std::function<void()>> & reduction_registry() {
    return reduction_registry_;
  } // reduction_registry

  /*--------------------------------------------------------------------------*
    Global object interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register a global object with the context.

    @tparam NAMESPACE_HASH A unique hash that identifies the object.
    @tparam INDEX          The index of the global object within the given
                           namespace.
    @tparam OBJECT_TYPE    The C++ type of the object.

    @return a boolean value indicating success or failure.
   */

  template<size_t NAMESPACE_HASH, size_t INDEX, typename OBJECT_TYPE>
  bool register_global_object() {
    size_t KEY = NAMESPACE_HASH ^ INDEX;

    using wrapper_t = global_object_wrapper_u<OBJECT_TYPE>;

    std::get<0>(global_object_registry_[KEY]) = {};
    std::get<1>(global_object_registry_[KEY]) = &wrapper_t::cleanup;

    return true;
  } // register_global_object

  /*!
    Register a global object with the context.

    @tparam NAMESPACE_HASH A unique hash that identifies the object.
    @tparam OBJECT_TYPE    The C++ type of the object.

    @param index The index of the global object within the given namespace.
    @param args  A variadic argument list to be passed to the constructor
                 of the global object.

    @return a boolean value indicating success or failure.
   */

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE, typename... ARGS>
  bool initialize_global_object(size_t index, ARGS &&... args) {
    size_t KEY = NAMESPACE_HASH ^ index;
    assert(global_object_registry_.find(KEY) != global_object_registry_.end());
    std::get<0>(global_object_registry_[KEY]) =
      reinterpret_cast<uintptr_t>(new OBJECT_TYPE(std::forward<ARGS>(args)...));
    return true;
  } // new_global_object

  /*!
    Get a global object instance.

    @tparam NAMESPACE_HASH A unique hash that identifies the object.
    @tparam OBJECT_TYPE    The C++ type of the object.

    @param index The index of the global object within the given namespace.

    @return a boolean value indicating success or failure.
   */

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE>
  OBJECT_TYPE * get_global_object(size_t index) {
    size_t KEY = NAMESPACE_HASH ^ index;
    assert(global_object_registry_.find(KEY) != global_object_registry_.end());
    return reinterpret_cast<OBJECT_TYPE *>(
      std::get<0>(global_object_registry_[KEY]));
  } // get_global_object

  /*--------------------------------------------------------------------------*
    Function interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register a funtion with the runtime. Internally, interface is used both
    for user-defined functions, and for task registration for MPI.

    @tparam KEY       A hash key identifying the.
    @tparam RETURN    The return type of the function.
    @tparam ARG_TUPLE A std::tuple of the function argument types.
    @tparam FUNCTION  A pointer to the function.

    @return a boolean indicating the succes or failure of the registration.
   */

  template<size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE)>
  bool register_function() {
    flog_assert(function_registry_.find(KEY) == function_registry_.end(),
      "function has already been registered");

    const std::size_t addr = reinterpret_cast<std::size_t>(FUNCTION);
    flog(info) << "Registering function: " << addr << std::endl;

    function_registry_[KEY] = reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

private:
  /*--------------------------------------------------------------------------*
    Singleton.
   *--------------------------------------------------------------------------*/

  context_u() : CONTEXT_POLICY() {}

  ~context_u() {

    // Cleanup the global objects
    for(auto & go : global_object_registry_) {
      std::get<1>(go.second)(std::get<0>(go.second));
    } // for
  } // ~context_u

  context_u(const context_u &) = delete;
  context_u & operator=(const context_u &) = delete;
  context_u(context_u &&) = delete;
  context_u & operator=(context_u &&) = delete;

  /*--------------------------------------------------------------------------*
    Basic runtime data members.
   *--------------------------------------------------------------------------*/

  size_t color_ = 0;
  size_t colors_ = 0;
  top_level_action_t top_level_action_ = {};

  /*--------------------------------------------------------------------------*
    Reduction data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, std::function<void()>> reduction_registry_;

  /*--------------------------------------------------------------------------*
    Global object data members.
   *--------------------------------------------------------------------------*/

  using global_object_data_t =
    std::pair<uintptr_t, std::function<void(uintptr_t)>>;

  std::unordered_map<size_t, global_object_data_t> global_object_registry_;

  /*--------------------------------------------------------------------------*
    Function data members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, void *> function_registry_;

}; // struct context_u

} // namespace execution
} // namespace flecsi

// This include file defines the FLECSI_RUNTIME_CONTEXT_POLICY used below.

#include <flecsi/runtime/flecsi_runtime_context_policy.h>

namespace flecsi {
namespace execution {

/*!
  The context_t type is the high-level interface to the FleCSI execution
  context.

  @ingroup execution
 */

using context_t = context_u<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi
