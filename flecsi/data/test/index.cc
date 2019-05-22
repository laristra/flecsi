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
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/ftest.h>

using namespace flecsi;

flecsi_add_index_field("test", "value", double, 2);
inline auto th = flecsi_index_field_instance("test", "value", double, 0);

#if 0
inline auto ih = flecsi_topology_reference(index_topology_t, "topos", "index");
inline auto fh0 = flecsi_field_reference(ih, "test", "value", double, 0);
inline auto fh1 = flecsi_field_reference(ih, "test", "value", double, 1);

namespace ns {

void update(index_topology_accessor<rw> topo, index_field_accessor<rw> f) {
}

}

int
test(int argc, char ** argv) {

  FTEST();

  std::vector<size_t> lmap;

  for(auto & i: lmap) {
    i = 0;
  } // for

  ih.set_coloring(coloring);
  ih.set_launch_map(lmap);

  auto ld = flecsi_launch_domain("ld", 2);

  flecsi_execute_task(update, ns, ld, ih, fh0);

  return 0;
}

ftest_register_test(test);
#endif
