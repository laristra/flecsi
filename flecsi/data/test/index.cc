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
inline auto fh = flecsi_index_field_instance("test", "value", double, 0);

template<size_t PRIVILEGES>
using accessor =
  flecsi::data::index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace index_test {

void
assign(accessor<rw> ia) {
  ia = flecsi_color();
} // assign

flecsi_register_task(assign, index_test, loc, index);

void
print(accessor<ro> ia) {
  flog(info) << "Value: " << ia << std::endl;
} // print

flecsi_register_task(print, index_test, loc, index);

} // namespace index_test

int
index_topology(int argc, char ** argv) {

  FTEST();
#if 0

  flecsi_execute_task(assign, index_test, index, fh);
  flecsi_execute_task(print, index_test, index, fh);
#endif
  return 0;
} // index

ftest_register_test(index_topology);
