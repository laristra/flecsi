#pragma once

/*! @file */

#include <algorithm>
#include <bitset>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <vector>

namespace flecsi {
namespace utils {

#ifndef FLECSI_NODE_ATTRIBUTE_BITS
#define FLECSI_NODE_ATTRIBUTE_BITS 64
#endif

/*!
  The node_t type provides an interface for nodes of a control DAG
  in the FleCSI control model.
 */

struct node_t
{
  using bitset_t = std::bitset<FLECSI_NODE_ATTRIBUTE_BITS>;
  using edge_list_t = std::list<size_t>;
  using action_t = std::function<int(int, char **)>;

  /*!
    Construct a node_t with identifier \em hash.

    @param hash The hashed name of the node.
   */

  node_t(size_t hash = 0, action_t const & action = {},
    bitset_t const & bitset = {})
    : hash_(hash), action_(action), bitset_(bitset) {}

  /*!
    Copy constructor.

    @param node The node.
   */

  node_t(node_t const & node)
    :
      hash_(node.hash_),
      action_(node.action_),
      bitset_(node.bitset_),
      edge_list_(node.edge_list_)
    {}

  size_t const & hash() const { return hash_; }
  size_t & hash() { return hash_; }

  bitset_t const & bitset() const { return bitset_; }
  bitset_t & bitset() { return bitset_; }

  edge_list_t const & edges() const { return edge_list_; }
  edge_list_t & edges() { return edge_list_; }

  action_t const & action() const { return action_; }
  action_t & action() { return action_; }

  node_t & operator = (node_t const & n) {
    hash_ = n.hash_;
    bitset_ = n.bitset_;
    action_ = n.action_;
    edge_list_ = n.edge_list_;
    return *this;
  } // operator =

  int operator () (int argc, char ** argv) { action_(argc, argv); }

private:

  size_t hash_;
  action_t action_;
  bitset_t bitset_;
  edge_list_t edge_list_;

}; // struct node_t

struct dag_t
{
  using node_map_t = std::map<size_t, node_t>;
  using node_vector_t = std::vector<node_t>;
  using node_list_t = std::list<node_t>;

  node_map_t const & nodes() const { return nodes_; }
  node_map_t & nodes() { return nodes_; }

  node_t & node(size_t hash) { return nodes_[hash]; }

  /*!
    This method updates the node state without overwriting any of the
    edge information. This is useful for allowing users to access the node
    from anywhere in the code in a way that does not destroy the graph.

    @param node A node with updated hash, bitset, and action state.

    @return A boolean value that can be used to capture registrations
            at file scope.
   */
  bool update_node(node_t const & node) {
    nodes_[node.hash()].hash() = node.hash();
    nodes_[node.hash()].bitset() = node.bitset();
    nodes_[node.hash()].action() = node.action();
    return true;
  } // add_node

  /*!
    This adds an edge to the graph. The edges are stored as edge
    dependencies, hence the semantic of \em to <- \em from.

    @param to   The 'to' side of the directed dependency.
    @param from The 'from' side of the directed dependency.

    @return A boolean value that can be used to capture additions
            at file scope.
   */

  bool add_edge(size_t to, size_t from) {

    // Add the 'to' node if it doesn't already exist
    if(nodes_.find(to) == nodes_.end()) {
      nodes_[to].hash() = to;
    } // if

    // Add the 'from' node if it doesn't already exist
    if(nodes_.find(from) == nodes_.end()) {
      nodes_[from].hash() = from;
    } // if

    nodes_[to].edges().push_back(from);
  } // add_edge

  /*!
   */

  node_vector_t sort() {
    node_vector_t sorted;

    // Create a list of the nodes
    node_list_t nodes;
    for(auto n = nodes_.begin(); n != nodes_.end(); ++n) {
      nodes.push_back(n->second);
    } // for

    // Create a tally of the number of dependencies of each node
    // in the graph.
    std::queue<node_t> q;

    for(auto n = nodes.begin(); n != nodes.end();) {
      if(n->edges().size() == 0) {
        q.push(*n);
        n = nodes.erase(n);
      }
      else {
        ++n;
      } // if
    } // for

    size_t count{0};
    while(!q.empty()) {
      auto root = q.front();
      sorted.push_back(root);
      q.pop();

      for(auto n = nodes.begin(); n != nodes.end();) {
        auto it = std::find(n->edges().begin(), n->edges().end(), root.hash());

        if(it != n->edges().end()) {
          n->edges().erase(it);

          if(!n->edges().size()) {
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

    if(count != nodes_.size()) {
      // error
    } // if

    return sorted;
  } // sort

private:

  node_map_t nodes_;

}; // struct dag_t

} // namespace utils
} // namespace flecsi
