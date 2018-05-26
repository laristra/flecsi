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

#include <functional>
#include <map>
#include <vector>

#include <flecsi/utils/dag.h>

namespace flecsi {
namespace control {

template<typename NODE_POLICY>
struct control__ {

  using dag_t = flecsi::utils::dag__<NODE_POLICY>;
  using node_t = typename dag_t::node_t;

  static control__ & instance() {
    static control__ c;
    return c;
  } // instance

  /*!
    Return the control map for the given phase.

    @param phase The control point id or \em phase. Phases are defined
                 by the specialization.
   */
  
  dag_t & phase_map(size_t phase) {
    return registry_[phase];
  } // control_phase_map

// FIXME

  std::vector<node_t> const & sorted_phase_map(size_t phase) {
    return sorted_[phase];
  } // sorted_phase_map

  void init() {
    for(auto & d: registry_) {
      sorted_[d.first] = d.second.sort();
    } // for
  } // init

private:

  std::map<size_t, dag_t> registry_;
  std::map<size_t, std::vector<node_t>> sorted_;

}; // control__

} // namespace flecsi
} // namespace control
