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

#include <flecsi/control/control_point_walker.hh>
#include <flecsi/utils/dag.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/flog.hh>

#include <functional>
#include <map>
#include <vector>

flog_register_tag(control);

namespace flecsi {
namespace control {

/*!
  The control type provides a control model for specifying a
  set of control points as a coarse-grained control flow graph,
  with each node of the graph specifying a set of actions as a
  directed acyclic graph (DAG). The actions under a control point
  DAG are topologically sorted to respect dependency edges, which can
  be specified through the dag interface.

  If Graphviz support is enabled, the control flow graph and its DAG nodes
  can be written to a graphviz file that can be compiled and viewed using
  the \em dot program.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct control : public CONTROL_POLICY {

  using dag_t = flecsi::utils::dag<typename CONTROL_POLICY::node_t>;
  using node_t = typename dag_t::node_t;
  using control_point_walker_t = control_point_walker<control<CONTROL_POLICY>>;

  /*!
    Meyer's singleton.
   */

  static control & instance() {
    static control c;
    return c;
  } // instance

  /*!
    Allow control points to set the exit status for execution.
   */

  int exit_status() const {
    return exit_status_;
  }

  /*!
    Allow control points to set the exit status for execution.
   */

  int & exit_status() {
    return exit_status_;
  }

  /*!
    Execute the control flow graph.

    @param argc The number of command-line arguments.
    @param argv The command-line arguments.

    @return An integer with \em 0 being success, and any other value
            being failure.
   */

  static int execute(int argc, char ** argv) {

    {
      flog_tag_guard(control);
      flog_devel(info) << "Invoking control model" << std::endl
                       << "\tpolicy type: "
                       << utils::demangle(typeid(CONTROL_POLICY).name())
                       << std::endl;
    }

    instance().sort_control_points();
    control_point_walker_t pw(argc, argv);
    pw.template walk_types<typename CONTROL_POLICY::control_points>();
    return instance().exit_status();
  } // execute

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  using control_point_writer_t = control_point_writer<control<CONTROL_POLICY>>;
  using graphviz_t = flecsi::utils::graphviz_t;

  /*!
    Write the control flow graph to the graphviz object \em gv.

    @param gv An instance of the graphviz_t type. Elements of the
              control flow graph will be written to this object, which
              may then be output to a file.
   */

  void write(graphviz_t & gv) {
    sort_control_points();
    control_point_writer_t pw(gv);
    pw.template walk_types<typename CONTROL_POLICY::control_points>();
  } // write
#endif // FLECSI_ENABLE_GRAPHVIZ

  /*!
    Return the control map for the given control point.

    @param control_point The control point id or \em control point. Control
                         points are defined by the specialization.
   */

  dag_t & control_point_map(size_t control_point,
    std::string const & label = "default") {
    if(registry_.find(control_point) == registry_.end()) {
      registry_[control_point].label() = label;
    } // if

    return registry_[control_point];
  } // control_point_map

  /*!
    Return the sorted control map for the given control point.

    @param control_point The control point id or \em control point. Control
                         points are defined by the specialization.
   */

  std::vector<node_t> const & sorted_control_point_map(size_t control_point) {
    return sorted_[control_point];
  } // sorted_control_point_map

private:
  /*
    Sort the dag under each control point.
   */

  void sort_control_points() {
    if(sorted_.size() == 0) {
      for(auto & d : registry_) {
        sorted_[d.first] = d.second.sort();
      } // for
    } // if
  } // sort_control_points

  /*--------------------------------------------------------------------------*
    Data members.
   *--------------------------------------------------------------------------*/

  int exit_status_ = 0;
  std::map<size_t, dag_t> registry_;
  std::map<size_t, std::vector<node_t>> sorted_;

}; // control

} // namespace control
} // namespace flecsi

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include <flecsi/runtime/runtime.hh>

/*!
  @def flecsi_register_control_options

  This macro registers a command-line option "--control-model" that,
  when invoked, outputs a graphviz dot file that can be used to
  visualize the control points and actions of an executable that
  uses the control type passed as an argument.

  @param control_type A qualified specialization of control.

  @ingroup execution
 */

#define flecsi_register_control_options(control_type)                          \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  using namespace boost::program_options;                                      \
                                                                               \
  inline void flecsi_control_add_options(options_description & desc) {         \
    desc.add_options()(                                                        \
      "control-model", "Output the current control model and exit.");          \
  }                                                                            \
                                                                               \
  inline int flecsi_control_initialize(                                        \
    int argc, char ** argv, variables_map & vm) {                              \
    if(vm.count("control-model")) {                                            \
      flecsi::utils::graphviz_t gv;                                            \
      control_type::instance().write(gv);                                      \
      auto file =                                                              \
        flecsi::utils::flog::rstrip<'/'>(argv[0]) + "-control-model.dot";      \
      std::cout << "Writing control model to " << file << std::endl;           \
      std::cout << "Execute:" << std::endl;                                    \
      std::cout << "\t$ dot -Tpdf " << file << " > model.pdf" << std::endl;    \
      std::cout << "to create a pdf image of the control model" << std::endl;  \
      gv.write(file.c_str());                                                  \
      return 1;                                                                \
    }                                                                          \
                                                                               \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  inline int flecsi_control_finalize(                                          \
    int argc, char ** argv, exit_mode_t mode) {                                \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  inline runtime_handler_t flecsi_control_handler{flecsi_control_initialize,   \
    flecsi_control_finalize,                                                   \
    flecsi_control_add_options};                                               \
                                                                               \
  flecsi_append_runtime_handler(flecsi_control_handler);

#else

#define flecsi_register_control_options(control_type)

#endif // FLECSI_ENABLE_GRAPHVIZ
