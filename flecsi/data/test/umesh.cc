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

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topology;

#if 0
using cell_field_t = field_member<double, dense, mesh_t, cells>;
const cell_field_t cell_field;
auto pressure = cell_field(m);
#endif

int
index_driver(int, char **) {

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
