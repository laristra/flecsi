#pragma once

#include <functional>
#include <map>

#include <dag.h>

namespace flecsi {
namespace execution {

struct context_t {

  using dag_t = utils::dag_t;
  using node_t = utils::node_t;

  static context_t & instance() {
    static context_t r;
    return r;
  } // context_t

  dag_t & phase_map(size_t phase) {
    return registry_[phase];
  } // phase

  std::vector<node_t> const & sorted_phase_map(size_t phase) {
    return sorted_[phase];
  } // phase

  size_t advance() { return ++step_; }

  bool run(size_t nsteps) {
    return step_<nsteps;
  }

  void init() {
    for(auto & d: registry_) {
      sorted_[d.first] = d.second.sort();
    } // for
  } // init

private:

  context_t() : step_(0) {}

  size_t step_;
  size_t nsteps_;

  std::map<size_t, dag_t> registry_;
  std::map<size_t, std::vector<node_t>> sorted_;

}; // struct context_t

} // namespace execution
} // namespace flecsi
