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

#include <functional>
#include <map>
#include <vector>

#include <flecsi-config.h>

namespace flecsi {
namespace control {

/*!
 */

template<typename CONTROL_POLICY>
struct control_u : public CONTROL_POLICY {

  using dag_t = flecsi::utils::dag_u<typename CONTROL_POLICY::node_t>;
  using node_t = typename dag_t::node_t;
  using phase_walker_t = phase_walker_u<control_u<CONTROL_POLICY>>;

  static control_u & instance() {
    static control_u c;
    return c;
  } // instance

  static int execute(int argc, char ** argv) {
    instance().sort_phases();
    phase_walker_t pw(argc, argv);
    pw.template walk_types<typename CONTROL_POLICY::phases>();
    return 0;
  } // execute

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  using phase_writer_t = phase_writer_u<control_u<CONTROL_POLICY>>;
  using graphviz_t = flecsi::utils::graphviz_t;

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
    Retrun the sorted control map for the given phase.

    @param phase The control point id or \em phase. Phases are defined
                 by the specialization.
   */

  std::vector<node_t> const & sorted_phase_map(size_t phase) {
    return sorted_[phase];
  } // sorted_phase_map

private:

  void sort_phases() {
    if(sorted_.size() == 0) {
      for(auto & d: registry_) {
        sorted_[d.first] = d.second.sort();
      } // for
    } // if
  } // sort_phases

  std::map<size_t, dag_t> registry_;
  std::map<size_t, std::vector<node_t>> sorted_;

}; // control_u

} // namespace flecsi
} // namespace control
