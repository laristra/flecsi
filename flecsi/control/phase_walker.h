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

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include <flecsi/utils/graphviz.h>
#endif

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

  using BEGIN_TYPE = typename std::tuple_element<0, TYPE>::type;
  using END_TYPE =
    typename std::tuple_element<std::tuple_size<TYPE>::value-1, TYPE>::type;

  static constexpr size_t begin = BEGIN_TYPE::value;
  static constexpr size_t end = END_TYPE::value;

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

    for(auto & node: sorted) {
      node.action()(argc_, argv_);
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

#if defined(FLECSI_ENABLE_GRAPHVIZ)

/*!
  The phase_writer__ class allows execution of statically-defined
  control points.
 */

template<typename CONTROL_POLICY>
struct phase_writer__
  : public flecsi::utils::tuple_walker__<phase_writer__<CONTROL_POLICY>>
{
  using graphviz_t = flecsi::utils::graphviz_t;

  phase_writer__(graphviz_t & gv) : gv_(gv) {}

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
    auto & phase_map = CONTROL_POLICY::instance().phase_map(PHASE_TYPE::value);

    // Add the control point node to the graph
    auto * root = gv_.add_node(phase_map.label().c_str(),
      phase_map.label().c_str());

    // Add edge dependency to last control point
    if(PHASE_TYPE::value > 0) {
      auto & last =
        CONTROL_POLICY::instance().phase_map(PHASE_TYPE::value - 1);
      auto * last_node = gv_.node(last.label().c_str());
      gv_.add_edge(last_node, root);
    } // if

    // Add graph to output
    phase_map.add(gv_);

    // Add edges from graph to control node
    auto & unsorted = phase_map.nodes();
    for(auto & n: unsorted) {
      if(n.second.edges().size() == 0) {
        gv_.add_edge(root, gv_.node(std::to_string(n.second.hash()).c_str()));
      } // if
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

    phase_writer__ phase_writer(gv_);
    phase_writer.template walk_types<typename PHASE_TYPE::TYPE>();

    if( PHASE_TYPE::begin != PHASE_TYPE::end) {
      auto & begin = CONTROL_POLICY::instance().phase_map(PHASE_TYPE::begin);
      auto & end = CONTROL_POLICY::instance().phase_map(PHASE_TYPE::end);
      auto * edge = gv_.add_edge(
        gv_.node(end.label().c_str()),
        gv_.node(begin.label().c_str())
      );
      gv_.set_edge_attribute(edge, "label", "cycle");
      gv_.set_edge_attribute(edge, "color", "#1d76db");
      gv_.set_edge_attribute(edge, "style", "bold");
      gv_.set_edge_attribute(edge, "headport", "e");
      gv_.set_edge_attribute(edge, "tailport", "e");
    } // if

  } // handle_type

private:

  graphviz_t & gv_;

}; // struct phase_writer__

#endif // FLECSI_ENABLE_GRAPHVIZ

} // namespace flecsi
} // namespace control
