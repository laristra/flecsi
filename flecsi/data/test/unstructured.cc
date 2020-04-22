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
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;

#if 0
const field<double>::definition<mesh_t,cells> cell_field;
auto pressure = cell_field(m);
#endif

int
unstructured_driver() {
  UNIT {
#if 0
  coloring.create(/* arg list */);

  mesh0.create(coloring);
  mesh1.create(coloring);

  execute<assign>(mesh0, pressure);
  execute<check>(pressure);

  mesh0.destroy();
  mesh1.destroy();
#endif
  };
} // index

flecsi::unit::driver<unstructured_driver> driver;
