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

#include <flecsi/utils/const_string.h>
#include <flecsi/utils/tuple_walker.h>
#include <flecsi/utils/typeify.h>

#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include <flecsi/utils/graphviz.h>
#endif

namespace flecsi {
namespace control {

/*!
  Convenience type for defining control points.
 */

template<size_t CONTROL_POINT>
using control_point_ = flecsi::utils::typeify_u<size_t, CONTROL_POINT>;

/*!
  Allow users to define cyclic control points. Cycles can be nested.

  @tparam PREDICATE      A predicate function that determines when
                         the cycle should end.
  @tparam CONTROL_POINTS A variadic list of control points within the cycle.

  @ingroup control
 */

template<bool (*PREDICATE)(), typename... CONTROL_POINTS>
struct cycle_u {

  using TYPE = std::tuple<CONTROL_POINTS...>;

  using BEGIN_TYPE = typename std::tuple_element<0, TYPE>::type;
  using END_TYPE =
    typename std::tuple_element<std::tuple_size<TYPE>::value - 1, TYPE>::type;

  static constexpr size_t begin = BEGIN_TYPE::value;
  static constexpr size_t end = END_TYPE::value;

  static bool predicate() {
    return PREDICATE();
  } // run

}; // struct cycle_u

/*!
  The control_point_walker_u class allows execution of statically-defined
  control points.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct control_point_walker_u : public flecsi::utils::tuple_walker_u<
                                  control_point_walker_u<CONTROL_POLICY>> {
  control_point_walker_u(int argc, char ** argv) : argc_(argc), argv_(argv) {}

  /*!
    Handle the tuple type \em ELEMENT_TYPE.

    @tparam ELEMENT_TYPE The tuple element type. This can either be a size_t
                         or a \em cycle. Cycles are defined by the
                         specialization and must conform to the interface used
                         in the appropriate visit_type method.
   */

  template<typename ELEMENT_TYPE>
  void visit_type() {

    if constexpr(std::is_same<typename ELEMENT_TYPE::TYPE, size_t>::value) {

      // This is not a cycle -> execute each control action for this
      // control point.
      auto & sorted = CONTROL_POLICY::instance().sorted_control_point_map(
        ELEMENT_TYPE::value);

      for(auto & node : sorted) {
        node.action()(argc_, argv_);
      } // for
    }
    else {

      // This is a cycle -> create a new control point walker to recurse
      // the cycle.
      while(ELEMENT_TYPE::predicate()) {
        control_point_walker_u control_point_walker(argc_, argv_);
        control_point_walker.template walk_types<typename ELEMENT_TYPE::TYPE>();
      } // while
    } // if

  } // visit_type

private:
  int argc_;
  char ** argv_;

}; // struct control_point_walker_u

#if defined(FLECSI_ENABLE_GRAPHVIZ)

/*!
  The control_point_writer_u class allows execution of statically-defined
  control points.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct control_point_writer_u : public flecsi::utils::tuple_walker_u<
                                  control_point_writer_u<CONTROL_POLICY>> {
  using graphviz_t = flecsi::utils::graphviz_t;

  control_point_writer_u(graphviz_t & gv) : gv_(gv) {}

  /*!
    Handle the tuple type \em ELEMENT_TYPE for type size_t.

    @tparam ELEMENT_TYPE The tuple element type. This can either be a size_t
                         or a \em cycle. Cycles are defined by the
                         specialization and must conform to the interface used
                         in the appropriate visit_type method.
   */

  template<typename ELEMENT_TYPE>
  typename std::enable_if<
    std::is_same<typename ELEMENT_TYPE::TYPE, size_t>::value>::type
  visit_type() {
    auto & control_point_map =
      CONTROL_POLICY::instance().control_point_map(ELEMENT_TYPE::value);

    // Add the control point node to the graph
    auto * root = gv_.add_node(
      control_point_map.label().c_str(), control_point_map.label().c_str());

    // Add edge dependency to last control point
    if(ELEMENT_TYPE::value > 0) {
      auto & last =
        CONTROL_POLICY::instance().control_point_map(ELEMENT_TYPE::value - 1);
      auto * last_node = gv_.node(last.label().c_str());
      gv_.add_edge(last_node, root);
    } // if

    // Add graph to output
    control_point_map.add(gv_);

    // Add edges from graph to control node
    auto & unsorted = control_point_map.nodes();
    for(auto & n : unsorted) {
      if(n.second.edges().size() == 0) {
        gv_.add_edge(root, gv_.node(std::to_string(n.second.hash()).c_str()));
      } // if
    } // for

  } // visit_type

  /*!
    Handle the tuple type \em ELEMENT_TYPE for type cycle_u.

    @tparam ELEMENT_TYPE The tuple element type. This can either be a size_t
                         or a \em cycle. Cycles are defined by the
                         specialization and must conform to the interface used
                         in the appropriate visit_type method.
   */

  template<typename ELEMENT_TYPE>
  typename std::enable_if<
    !std::is_same<typename ELEMENT_TYPE::TYPE, size_t>::value>::type
  visit_type() {

    control_point_writer_u control_point_writer(gv_);
    control_point_writer.template walk_types<typename ELEMENT_TYPE::TYPE>();

    // Add edges for cycles and beautify them...

    if(ELEMENT_TYPE::begin != ELEMENT_TYPE::end) {
      auto & begin =
        CONTROL_POLICY::instance().control_point_map(ELEMENT_TYPE::begin);
      auto & end =
        CONTROL_POLICY::instance().control_point_map(ELEMENT_TYPE::end);
      auto * edge = gv_.add_edge(
        gv_.node(end.label().c_str()), gv_.node(begin.label().c_str()));
      gv_.set_edge_attribute(edge, "label", "cycle");
      gv_.set_edge_attribute(edge, "color", "#1d76db");
      gv_.set_edge_attribute(edge, "fillcolor", "#1d76db");
      gv_.set_edge_attribute(edge, "style", "bold");
      gv_.set_edge_attribute(edge, "headport", "e");
      gv_.set_edge_attribute(edge, "tailport", "e");
    } // if
  } // visit_type

private:
  graphviz_t & gv_;

}; // struct control_point_writer_u

#endif // FLECSI_ENABLE_GRAPHVIZ

} // namespace control
} // namespace flecsi
