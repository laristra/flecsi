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

#include <flecsi/execution/context.h>
#include <flecsi/utils/tuple_walker.h>

namespace flecsi {
namespace control {

/*!
  Allow users to define cyclic control points. Cycles can be nested.

  @tparam PREDICATE  A predicate function that determines when
                     the cycle should end.
  @tparam PHASES ... A variadic list of phases within the cycle.
 */

template<bool (*PREDICATE)(), typename ... PHASES>
struct cycle__ {

  using TYPE = std::tuple<PHASES ...>;

  static bool predicate() {
    return PREDICATE();
  } // run

}; // struct cycle__

/*!
  The phase_walker__ class allows execution of statically-defined
  control points.
 */

template<typename CONTROL_POLICY>
struct phase_walker__
  : public flecsi::utils::tuple_walker__<phase_walker__<CONTROL_POLICY>>
{
  phase_walker__(int argc, char ** argv) : argc_(argc), argv_(argv) {}

  /*!
    Handle the tuple type \em PHASE_TYPE for type size_t.

    @tparam PHASE_TYPE The phase type. This can either be a size_t
                       or a \em cycle. Cycles are defined by the
                       specialization and must conform to the
                       interface used in the appropriate handle_type
                       method.
   */

  template<typename PHASE_TYPE>
  typename std::enable_if<
    std::is_same<typename PHASE_TYPE::TYPE, size_t>::value>::type
  handle_type() {
    auto & context = flecsi::execution::context_t::instance();

    // Execute each control action for this phase
    auto & sorted =
      CONTROL_POLICY::instance().sorted_phase_map(PHASE_TYPE::value);

    for(auto & entry: sorted) {
      entry.action()(argc_, argv_);
    } // for
  } // handle_type

  /*!
    Handle the tuple type \em PHASE_TYPE for type cycle__.

    @tparam PHASE_TYPE The phase type. This can either be a size_t
                       or a \em cycle. Cycles are defined by the
                       specialization and must conform to the
                       interface used in the appropriate handle_type
                       method.
   */

  template<typename PHASE_TYPE>
  typename std::enable_if<
    !std::is_same<typename PHASE_TYPE::TYPE, size_t>::value>::type
  handle_type() {
    while(PHASE_TYPE::predicate()) {
      phase_walker__ phase_walker(argc_, argv_);
      phase_walker.template walk_types<typename PHASE_TYPE::TYPE>();
    } // while
  } // handle_type

private:

  int argc_;
  char ** argv_;

}; // struct phase_walker__

} // namespace flecsi
} // namespace control
