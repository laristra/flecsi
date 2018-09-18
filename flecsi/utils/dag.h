#pragma once

/*! @file */

#include <list>
#include <map>
#include <queue>
#include <vector>

#include <cinchlog.h>

#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_GRAPHVIZ)
#include <flecsi/utils/graphviz.h>
#endif

namespace flecsi {
namespace utils {

/*!
  The dag_node__ type defines a compile-time extensible node for the
  FleCSI dag__ data structure.
 */

template<typename NODE_POLICY>
struct dag_node__ : public NODE_POLICY {

  using edge_list_t = std::list<size_t>;

  /*! Constructor */

  dag_node__() : NODE_POLICY() {}

  /*!
    Constructor.

    @param hash  The node identifier.
    @param label The string label of the node.
    @param ARGS  A variadic list of arguments that are passed to
                 the node policy constructor.
   */

  template<typename ... ARGS>
  dag_node__(size_t hash, std::string const & label, ARGS && ... args)
    : hash_(hash), label_(label), NODE_POLICY(std::forward<ARGS>(args) ...) {
  }

  /*!
    Copy constructor.
   */

  dag_node__(dag_node__ const & node)
    :
      hash_(node.hash_),
      label_(node.label_),
      edge_list_(node.edge_list_),
      NODE_POLICY(node)
    {}

  size_t const & hash() const { return hash_; }
  size_t & hash() { return hash_; }

  std::string const & label() const { return label_; }
  std::string & label() { return label_; }

  edge_list_t const & edges() const { return edge_list_; }
  edge_list_t & edges() { return edge_list_; }

  /*!
    This method initializes the node state without overwriting any of the
    edge information. This is useful for allowing users to access the node
    from anywhere in the code in a way that does not destroy the graph.

    @param node The node.

    @return A boolean value that can be used to capture registrations
            at file scope.
   */

  bool initialize(dag_node__ const & node) {
    hash_ = node.hash_;
    label_ = node.label_;
    return NODE_POLICY::initialize(node);
  } // update

  dag_node__ & operator = (dag_node__ const & node) {
    NODE_POLICY::operator = (node);

    hash_ = node.hash_;
    label_ = node.label_;
    edge_list_ = node.edge_list_;

    return *this;
  } // operator =

private:

  size_t hash_ = 0;
  std::string label_ = "";
  edge_list_t edge_list_ = {};

}; // struct dag_node__

template<typename NODE_POLICY>
inline std::ostream & 
operator << (std::ostream & stream, dag_node__<NODE_POLICY> const & node) {
  stream << "hash: " << node.hash() << std::endl;
  stream << "label: " << node.label() << std::endl;
  stream << static_cast<NODE_POLICY const &>(node);

  stream << "edges: ";
  for(auto e: node.edges()) {
    stream << e << " ";
  } // for
  stream << std::endl;

  return stream;
} // operator <<

template<typename NODE_POLICY>
struct dag__
{
  using node_t = dag_node__<NODE_POLICY>;
  using node_map_t = std::map<size_t, node_t>;
  using node_vector_t = std::vector<node_t>;
  using node_list_t = std::list<node_t>;

  std::string const & label() const { return label_; }
  std::string & label() { return label_; }

  node_map_t const & nodes() const { return nodes_; }
  node_map_t & nodes() { return nodes_; }

  node_t & node(size_t hash) { return nodes_[hash]; }

  bool initialize_node(node_t const & node) {
    return nodes_[node.hash()].initialize(node);
  } // initialize_node

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

	 return true;
  } // add_edge

  /*!
    Topological sort of the DAG using Kahn's algorithm.

    @return A std::vector<node_t> with a node ordering that respects
            the DAG dependencies.
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
      clog_fatal("sorting failed. This is not a DAG!!!");
    } // if

    return sorted;
  } // sort

#if defined(FLECSI_ENABLE_GRAPHVIZ)

  /*!
    Add the DAG to the graphviz graph.
   */

  void add(graphviz_t & gv) {
    std::map<size_t, Agnode_t *> node_map;

    for(auto n: nodes_) {
      const std::string hash = std::to_string(n.second.hash());

      node_map[n.second.hash()] =
        gv.add_node(hash.c_str(), n.second.label().c_str());

      gv.set_node_attribute(hash.c_str(), "color", "#c5def5");
      gv.set_node_attribute(hash.c_str(), "style", "filled");
      gv.set_node_attribute(hash.c_str(), "fillcolor", "#c5def5");
    } // for

    for(auto n: nodes_) {
      for(auto e: n.second.edges()) {
        gv.add_edge(node_map[e], node_map[n.first]);
      } // for
    } // for
  } // add_to_graphviz

#endif // FLECSI_ENABLE_GRAPHVIZ

private:

  std::string label_;
  node_map_t nodes_;

}; // struct dag__

template<typename NODE_POLICY>
inline std::ostream & 
operator << (std::ostream & stream, dag__<NODE_POLICY> const & dag) {
  for(auto n: dag.nodes()) {
    stream << n.second << std::endl;
  } // for

  return stream;
} // operator <<

} // namespace utils
} // namespace flecsi
