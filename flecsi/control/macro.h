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

#include <flecsi/control/phase_walker.h>

/*!
  @def flecsi_register_control_action

  Register a control point action with the control model.

  @param phase  The control point id or \em phase at which to register
                the given action.
  @param name   The name of the action. This value will be used to create
                a hash and should be unique to the phase.
  @param action The action to execute.
  @param ...    A variadic list of arguments to pass to the node that is
                created by the call. These are defined by the user's
                dag__ specialization type.

  @ingroup control
 */

#define flecsi_register_control_action(phase, name, action, ...)               \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  bool name##phase##_registered =                                              \
    flecsi::execution::context_t::instance().control_phase_map(phase).         \
      initialize_node(                                                         \
        { flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),    \
        action, ##__VA_ARGS__ })

/*!
  @def flecsi_register_control_action_edge

  Create a dependency between two control actions. The name arguments
  in this macro are those used to register the control actions.

  @param phase The control point id or \em phase at which to register
               the given action.
  @param to    The name of the downstream node in the edge.
  @param from  The name of the upstream node in the edge.

  @note Because this call should be invoked at file scope, the \em to
        and \em from names do not need to have been registered \em before
        edges can be added. There is no \em before at file scope...

  @ingroup control
 */

#define flecsi_add_control_action_edge(phase, to, from)                        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  bool registered_##to##from =                                                 \
    flecsi::execution::context_t::instance().control_phase_map(phase).         \
      add_edge(flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(to)}.hash(), \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(from)}.hash())
