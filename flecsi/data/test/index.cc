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
#include <flecsi/data/data.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

#if 0
namespace {
const data::field_interface_t::field<double> ifld(2);
const auto fh = ifld(flecsi_index_topology);
} // namespace

namespace new_interface {

using namespace flecsi::data;
using namespace flecsi::topology;

using dense_field_t = data::
  field_member_u<double, dense, index_topology_t, 0 /* index space */>;

const dense_field_t nifld(1);
const auto nifh = nifld(flecsi_index_topology);
} // namespace new_interface

template<size_t PRIVILEGES>
using accessor =
  flecsi::data::index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace index_test {

void
assign(accessor<rw> ia) {
  flog(info) << "assign on " << flecsi_color() << std::endl;
  ia = flecsi_color();
} // assign

flecsi_register_task(assign, index_test, loc, index);

int
check(accessor<ro> ia) {

  FTEST();

  flog(info) << "check on " << flecsi_color() << std::endl;
  ASSERT_EQ(ia, flecsi_color());

  return FTEST_RESULT();
} // print

flecsi_register_task(check, index_test, loc, index);

} // namespace index_test
#endif

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topology;

template<size_t PRIVILEGES>
using dense_accessor = index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

using dense_field_t = field_member_u<double, dense, index_topology_t, 0 /* index space */>;

const dense_field_t nifld(1);
const auto nifh = nifld(flecsi_index_topology);

void
assign(dense_accessor<rw> ia) {
  flog(info) << "assign on " << color() << std::endl;
  ia = color();
} // assign

int
check(dense_accessor<ro> ia) {

  FTEST();

  flog(info) << "check on " << color() << std::endl;
  ASSERT_EQ(ia, color());

  return FTEST_RESULT();
} // print

int
index_topology(int argc, char ** argv) {

  execute<assign>(nifh);
  execute<check>(nifh);

  return 0;
} // index

ftest_register_driver(index_topology);
