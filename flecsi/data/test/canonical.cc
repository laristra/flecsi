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
#include <flecsi/data.hh>
#include <flecsi/execution.hh>
#include <flecsi/utils/ftest.hh>

#include <tuple>

using namespace flecsi;

struct policy {
  constexpr static size_t value = 12;

  template<size_t VALUE>
  using typeify = flecsi::utils::typeify<size_t, VALUE>;

  enum index_spaces_t { vertices, cells }; // enum index_spaces_t

  using entity_types = std::tuple<typeify<vertices>, typeify<cells>>;

}; // struct policy

using canonical_topology =
  data::topology_reference<topology::canonical_topology<policy>>;
canonical_topology canonical;

canonical_topology::coloring coloring;

#if 0
using cell_field_t =
  data::field_member<double, data::dense, canonical_topology, policy::cells>;
const cell_field_t cell_field;
auto pressure = cell_field(canonical);
#endif

int
index_driver(int argc, char ** argv) {

  coloring.allocate();
  canonical.allocate(coloring);

  return 0;
} // index

ftest_register_driver(index_driver);
