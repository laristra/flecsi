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
#include <flecsi/topology/structured/interface.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

struct policy {

  using coloring = flecsi::topology::structured_base::coloring;

  static coloring color() {
    coloring c = 10;
    return c;
  } // color

}; // struct policy

using topology_type = topology::structured<policy>;

using structured_topology = data::topology_slot<topology_type>;
structured_topology structured;

data::coloring_slot<topology_type> coloring;

#if 0
using cell_field_t =
  data::field_member<double, data::dense, topology_type, policy::cells>;
const cell_field_t cell_field;
auto pressure = cell_field(structured);
#endif

int
check() {
  FTEST();

  flog(info) << "check" << std::endl;

  return FTEST_RESULT();
} // check

int
structured_driver(int, char **) {

  coloring.allocate();
  // structured.allocate(coloring.get());

  execute<check>();

  return 0;
} // index

ftest_register_driver(structured_driver);
