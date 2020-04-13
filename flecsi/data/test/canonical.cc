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

#define __FLECSI_PRIVATE__
#include "flecsi/topo/canonical/interface.hh"
#include "flecsi/util/ftest.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

#include <tuple>

using namespace flecsi;

struct policy {
  constexpr static size_t value = 12;

  template<size_t VALUE>
  using typeify = util::typeify<size_t, VALUE>;

  enum index_space { vertices, cells };
  static constexpr std::size_t index_spaces = 2;

  using entity_types = std::tuple<typeify<vertices>, typeify<cells>>;

  using coloring = topo::canonical_base::coloring;

  static coloring color(std::string const &) {
    coloring c;
    return c;
  } // color

}; // struct policy

using topology_type = topo::canonical<policy>;

using canonical_topology = data::topology_slot<topology_type>;
canonical_topology canonical;

data::coloring_slot<topology_type> coloring;

using cell_field_t =
  data::field_member<double, data::dense, topology_type, policy::cells>;
const cell_field_t cell_field;
auto pressure = cell_field(canonical);

int
check() {
  FTEST { flog(info) << "check" << std::endl; };
} // check

int
canonical_driver(int, char **) {
  FTEST {
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());

    EXPECT_EQ(test<check>(), 0);
  };
} // index

ftest_register_driver(canonical_driver);
