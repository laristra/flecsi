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

#include "flecsi/flog.hh"

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include "flecsi/util/graphviz.hh"
#endif

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <sstream>
#include <vector>

namespace flecsi {
namespace util {

namespace dag_impl {

/*
 */

template<typename NodePolicy>
struct node : NodePolicy, std::list<node<NodePolicy> const *> {

  template<typename... Args>
  node(std::string const & label, Args &&... args)
    : NodePolicy(std::forward<Args>(args)...), label_(label) {
    const void * address = static_cast<const void *>(this);
    std::stringstream ss;
    ss << address;
    identifier_ = ss.str();
  }

  std::string const & identifier() const {
    return identifier_;
  }

  std::string const & label() const {
    return label_;
  }

private:
  std::string identifier_;
  std::string label_;
};

} // namespace dag_impl

/*!
  Basic DAG type.
 */

template<typename NodePolicy>
struct dag : std::vector<dag_impl::node<NodePolicy> *> {

  using node_type = dag_impl::node<NodePolicy>;

  dag(const char * label = "empty") : label_(label) {}

  /*!
    @return the DAG label.
   */

  std::string const & label() const {
    return label_;
  }

  /*!
    Topological sort using Kahn's algorithm.

    @return A valid sequence of the nodes in the DAG.
   */

  std::vector<node_type const *> sort() {
    std::vector<node_type const *> sorted;

    // Create a temporary list of the nodes.
    std::list<node_type *> nodes;
    for(auto n : *this) {
      nodes.push_back(n);
    } // for

    // Create a tally of the number of dependencies of each node
    // in the graph. Remove nodes from the temporary list that do not
    // have any depenendencies.
    std::queue<node_type *> q;
    for(auto n = nodes.begin(); n != nodes.end();) {
      if((*n)->size() == 0) {
        q.push(*n);
        n = nodes.erase(n);
      }
      else {
        ++n;
      } // if
    } // for

    size_t count{0};
    while(!q.empty()) {
      const auto root = q.front();
      sorted.push_back(root);
      q.pop();

      for(auto n = nodes.begin(); n != nodes.end();) {
        auto it = std::find_if((*n)->begin(),
          (*n)->end(),
          [&root](const auto p) { return root == p; });

        if(it != (*n)->end()) {
          (*n)->erase(it);

          if(!(*n)->size()) {
            q.push(*n);
            n = nodes.erase(n);
          } // if
        }
        else {
          ++n;
        } // if
      } // for

      ++count;
    } // while

    flog_assert(count == this->size(), "sorting failed. This is not a DAG!!!");

    return sorted;
  } // sort

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  /*!
    Add the DAG to the given graphviz graph.
   */

  void add(graphviz & gv, const char * color = "#c5def5") {
    std::map<uintptr_t, Agnode_t *> node_map;

    for(auto n : *this) {

      auto * node = gv.add_node(n->identifier().c_str(), n->label().c_str());
      node_map[uintptr_t(n)] = node;

      gv.set_node_attribute(node, "color", "black");
      gv.set_node_attribute(node, "style", "filled");
      gv.set_node_attribute(node, "fillcolor", color);
    } // for

    for(auto n : *this) {
      for(auto e : *n) {
        auto * edge =
          gv.add_edge(node_map[uintptr_t(e)], node_map[uintptr_t(n)]);
        gv.set_edge_attribute(edge, "penwidth", "1.5");
      } // for
    } // for
  } // add

#endif // FLECSI_ENABLE_GRAPHVIZ

private:
  std::string label_;
}; // struct dag

} // namespace util
} // namespace flecsi
