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
using namespace flecsi::data;
using namespace flecsi::topology;

struct policy_t {
  constexpr static size_t value = 12;

  template<size_t VALUE>
  using typeify = flecsi::utils::typeify<size_t, VALUE>;

  enum index_spaces_t { vertices, cells }; // enum index_spaces_t

  using entity_types = std::tuple<typeify<vertices>, typeify<cells>>;

}; // struct policy_t

using canonical_topology_t =
  topology_reference<topology::canonical_topology<policy_t>>;
canonical_topology_t canonical;

canonical_topology_t::coloring_t coloring;

#if 0

using cell_field_t =
  field_member<double, dense, canonical_topology_t, policy_t::cells>;
const cell_field_t cell_field;
auto pressure = cell_field(canonical);

#endif

int
index_driver(int argc, char ** argv) {

  coloring.allocate();
  canonical.allocate(coloring);

#if 0
  coloring0.create(/* arg list */);

  mesh0.create(coloring0);
  mesh1.create(coloring0);

  execute<assign>(mesh0, pressure);
  execute<check>(pressure);

  mesh0.destroy();
  mesh1.destroy();
#endif

  return 0;
} // index

ftest_register_driver(index_driver);
