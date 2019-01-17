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
#include <flecsi/utils/dag.h>
#include <flecsi/utils/flog.h>

#include <functional>
#include <map>
#include <vector>

#include <flecsi-config.h>

flog_register_tag(control);

namespace flecsi {
namespace control {

/*!
  The control_u type provides a control model for specifying a
  set of control points as a coarse-grained control flow graph,
  with each node of the graph specifying a set of actions as a
  directed acyclic graph (DAG). The actions under a control point
  DAG are topologically sorted to respect dependency edges, which can
  be specified through the dag_u interface.

  If Graphviz support is enabled, the control flow graph and its DAG nodes
  can be written to a graphviz file that can be compiled and viewed using
  the \em dot program.

  @ingroup control
 */

template<typename CONTROL_POLICY>
struct control_u : public CONTROL_POLICY {

  using dag_t = flecsi::utils::dag_u<typename CONTROL_POLICY::node_t>;
  using node_t = typename dag_t::node_t;
  using phase_walker_t = phase_walker_u<control_u<CONTROL_POLICY>>;

  /*!
    Meyer's singleton.
   */

  static control_u & instance() {
    static control_u c;
    return c;
  } // instance

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
      flog(info) << __FUNCTION__ << std::endl;
    }

    instance().sort_phases();
    phase_walker_t pw(argc, argv);
    pw.template walk_types<typename CONTROL_POLICY::phases>();
    return 0;
  } // execute

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  using phase_writer_t = phase_writer_u<control_u<CONTROL_POLICY>>;
  using graphviz_t = flecsi::utils::graphviz_t;

  /*!
    Write the control flow graph to the graphviz object \em gv.

    @param gv An instance of the graphviz_t type. Elements of the
              control flow graph will be written to this object, which
              may then be output to a file.
   */

  void write(graphviz_t & gv) {
    sort_phases();
    phase_writer_t pw(gv);
    pw.template walk_types<typename CONTROL_POLICY::phases>();
  } // write
#endif

  /*!
    Return the control map for the given phase.

    @param phase The control point id or \em phase. Phases are defined
                 by the specialization.
   */

  dag_t & phase_map(size_t phase, std::string const & label = "default") {
    if(registry_.find(phase) == registry_.end()) {
      registry_[phase].label() = label;
    } // if

    return registry_[phase];
  } // control_phase_map

  /*!
    Return the sorted control map for the given phase.

    @param phase The control point id or \em phase. Phases are defined
                 by the specialization.
   */

  std::vector<node_t> const & sorted_phase_map(size_t phase) {
    return sorted_[phase];
  } // sorted_phase_map

private:
  /*
    Sort the dag under each control point.
   */

  void sort_phases() {
    if(sorted_.size() == 0) {
      for(auto & d : registry_) {
        sorted_[d.first] = d.second.sort();
      } // for
    } // if
  } // sort_phases

  /*--------------------------------------------------------------------------*
    Data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, dag_t> registry_;
  std::map<size_t, std::vector<node_t>> sorted_;

}; // control_u

} // namespace control
} // namespace flecsi
