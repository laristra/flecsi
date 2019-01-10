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

#include <flecsi/utils/flog.h>

#include <cstddef>
#include <functional>

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

  top_level_action_t & top_level_action() {
    return top_level_action_;
  } // top_level_action

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  /*!
    FIXME: This interface needs to be updated.
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
  size_t color_ = 0;
  size_t colors_ = 0;
  top_level_action_t top_level_action_ = {};

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, void *> function_registry_;

  //--------------------------------------------------------------------------//
  // Singleton.
  //--------------------------------------------------------------------------//

  context_u() : CONTEXT_POLICY() {}

  ~context_u() {} // ~context_u

  // These are deleted because this type is a singleton, i.e.,
  // we don't want anyone to be able to make copies or references.

  context_u(const context_u &) = delete;
  context_u & operator=(const context_u &) = delete;
  context_u(context_u &&) = delete;
  context_u & operator=(context_u &&) = delete;

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
