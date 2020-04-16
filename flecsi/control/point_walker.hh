/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#include <flecsi/utils/tuple_walker.hh>
#include <flecsi/utils/typeify.hh>

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include <flecsi/utils/graphviz.hh>
#endif

namespace flecsi {
namespace control {

/*!
  Convenience type for defining control points.
 */

template<size_t POINT>
using point_ = utils::typeify<size_t, POINT>;

/*!
  Allow users to define cyclic control points. Cycles can be nested.

  @tparam PREDICATE      A predicate function that determines when
                         the cycle should end.
  @tparam POINTS         A variadic list of control points within the cycle.

  @ingroup control
 */

template<bool (*PREDICATE)(), typename... POINTS>
struct cycle {

  using TYPE = std::tuple<POINTS...>;

  using BEGIN_TYPE = typename std::tuple_element<0, TYPE>::type;
  using END_TYPE =
    typename std::tuple_element<std::tuple_size<TYPE>::value - 1, TYPE>::type;

  static constexpr size_t begin = BEGIN_TYPE::value;
  static constexpr size_t end = END_TYPE::value;

  static bool predicate() {
    return PREDICATE();
  } // run

}; // struct cycle

/*!
  The point_walker class allows execution of statically-defined
  control points.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct point_walker : public utils::tuple_walker<point_walker<CONTROL_POLICY>> {
  point_walker(int argc, char ** argv) : argc_(argc), argv_(argv) {}

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

      // This is not a cycle -> execute each action for this control point.
      auto & sorted = CONTROL_POLICY::instance().sorted_control_point_map(
        ELEMENT_TYPE::value);
      int status{0};

      for(auto & node : sorted) {
        status |= node.action()(argc_, argv_);
      } // for

      CONTROL_POLICY::instance().exit_status() |= status;
    }
    else {

      // This is a cycle -> create a new control point walker to recurse
      // the cycle.
      while(ELEMENT_TYPE::predicate()) {
        point_walker point_walker(argc_, argv_);
        point_walker.template walk_types<typename ELEMENT_TYPE::TYPE>();
      } // while
    } // if

  } // visit_type

private:
  int argc_;
  char ** argv_;

}; // struct point_walker

#if defined(FLECSI_ENABLE_GRAPHVIZ)

/*!
  The point_writer class allows execution of statically-defined
  control points.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct point_writer
  : public flecsi::utils::tuple_walker<point_writer<CONTROL_POLICY>> {
  using graphviz_t = flecsi::utils::graphviz_t;

  point_writer(graphviz_t & gv) : gv_(gv) {}

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
    auto & point_map =
      CONTROL_POLICY::instance().point_map(ELEMENT_TYPE::value);

    // Add the control point node to the graph
    auto * root =
      gv_.add_node(point_map.label().c_str(), point_map.label().c_str());

    // Add edge dependency to last control point
    if(ELEMENT_TYPE::value > 0) {
      auto & last =
        CONTROL_POLICY::instance().point_map(ELEMENT_TYPE::value - 1);
      auto * last_node = gv_.node(last.label().c_str());
      gv_.add_edge(last_node, root);
    } // if

    // Add graph to output
    point_map.add(gv_);

    // Add edges from graph to control node
    auto & unsorted = point_map.nodes();
    for(auto & n : unsorted) {
      if(n.second.edges().size() == 0) {
        gv_.add_edge(root, gv_.node(std::to_string(n.second.hash()).c_str()));
      } // if
    } // for

  } // visit_type

  /*!
    Handle the tuple type \em ELEMENT_TYPE for type cycle.

    @tparam ELEMENT_TYPE The tuple element type. This can either be a size_t
                         or a \em cycle. Cycles are defined by the
                         specialization and must conform to the interface used
                         in the appropriate visit_type method.
   */

  template<typename ELEMENT_TYPE>
  typename std::enable_if<
    !std::is_same<typename ELEMENT_TYPE::TYPE, size_t>::value>::type
  visit_type() {

    point_writer point_writer(gv_);
    point_writer.template walk_types<typename ELEMENT_TYPE::TYPE>();

    // Add edges for cycles and beautify them...

    if(ELEMENT_TYPE::begin != ELEMENT_TYPE::end) {
      auto & begin = CONTROL_POLICY::instance().point_map(ELEMENT_TYPE::begin);
      auto & end = CONTROL_POLICY::instance().point_map(ELEMENT_TYPE::end);
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

}; // struct point_writer

#endif // FLECSI_ENABLE_GRAPHVIZ

} // namespace control
} // namespace flecsi
