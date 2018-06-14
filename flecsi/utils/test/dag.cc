/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <bitset>

#include <cinchtest.h>

#include <flecsi-config.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/dag.h>

struct node_policy_t {

  using bitset_t = std::bitset<8>;

  node_policy_t() {}

  node_policy_t(bitset_t const & bitset)
    : bitset_(bitset) {}

  bool initialize(node_policy_t const & node) {
    bitset_ = node.bitset_;
    return true;
  } // initialize

  bitset_t const & bitset() const { return bitset_; }
  bitset_t & bitset() { return bitset_; }

private:

  bitset_t bitset_;

}; // struct node_policy_t

inline std::ostream &
operator << (std::ostream & stream, node_policy_t const & node) {
  stream << "bitset: " << node.bitset() << std::endl;
  return stream;
} // operator <<

using dag_t = flecsi::utils::dag__<node_policy_t>;

#if defined(FLECSI_ENABLE_GRAPHVIZ)
using graphviz_t = flecsi::utils::graphviz_t;
#endif

#define flecsi_hash(name) \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()

const size_t a = flecsi_hash(a);
const size_t b = flecsi_hash(b);
const size_t c = flecsi_hash(c);
const size_t d = flecsi_hash(d);
const size_t e = flecsi_hash(e);
const size_t f = flecsi_hash(f);
const size_t g = flecsi_hash(g);

TEST(dag, sanity) {

  dag_t dag;

  dag.initialize_node({ e, "e", 0x10 | 0x20 }); 
  dag.initialize_node({ f, "f", 0x20 | 0x40 }); 
  dag.initialize_node({ g, "g", 0x40 | 0x80 }); 

  dag.add_edge(b, a);
  dag.add_edge(c, a);
  dag.add_edge(c, b);
  dag.add_edge(e, d);

  dag.initialize_node({ a, "a", 0x01 | 0x02 }); 
  dag.initialize_node({ b, "b", 0x02 | 0x04 }); 
  dag.initialize_node({ c, "c", 0x04 | 0x08 }); 
  dag.initialize_node({ d, "d", 0x08 | 0x10 }); 

  dag.add_edge(e, b);
  dag.add_edge(e, f);
  dag.add_edge(b, f);
  dag.add_edge(d, c);
  dag.add_edge(d, a);
  dag.add_edge(g, e);

  std::cout << dag << std::endl;

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  graphviz_t gv;
  dag.add(gv);
  gv.write("dag.gv");
#endif

} // TEST
