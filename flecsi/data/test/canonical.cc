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

  enum index_spaces_t {
    vertices,
    cells
  }; // enum index_spaces_t

  using entity_types = std::tuple<
    typeify<vertices>,
    typeify<cells>
  >;

}; // struct policy_t

using topo_t = topology::canonical_topology<policy_t>;

using canonical_definition = topology_need_name<topo_t>;
const canonical_definition canon;
auto canon0 = canon();

#if 0
using mesh = topology<mesh_t>;
const mesh mesh0;
const mesh mesh1;

const mesh::coloring coloring;

using cell_field_t = field_member<double, dense, mesh_t, cells>;
const cell_field_t cell_field;
auto pressure = cell_field(m);
#endif

int
index_driver(int argc, char ** argv) {

#if 0
  coloring.create(/* arg list */);

  mesh0.create(coloring);
  mesh1.create(coloring);

  execute<assign>(mesh0, pressure);
  execute<check>(pressure);

  mesh0.destroy();
  mesh1.destroy();
#endif

  return 0;
} // index

ftest_register_driver(index_driver);
