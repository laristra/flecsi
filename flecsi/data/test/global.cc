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

namespace {
static_assert(std::is_same_v<core_t<global>, global>);
struct test_g : global {};
static_assert(std::is_same_v<core_t<test_g>, global>);

struct test_policy {
  static constexpr std::size_t num_dimensions = 1, num_domains = 1;
  typedef std::tuple<> entity_types, connectivities, bindings;
  static void create_entity();
};

using u_base = unstructured<test_policy>;
static_assert(std::is_same_v<core_t<u_base>, u_base>);
struct test_u : u_base {};
static_assert(std::is_same_v<core_t<test_u>, u_base>);
} // namespace

using global_field_t = global_field_member<double>;
const global_field_t energy_field;

const auto energy = energy_field(global_topology);

void
assign(global_field_t::accessor<wo> ga) {
  flog(info) << "assign on " << color() << std::endl;
  ga = color();
} // assign

int
check(global_field_t::accessor<ro> ga) {
  FTEST {
    flog(info) << "check on " << color() << std::endl;
    ASSERT_EQ(ga, 0);
  };
} // check

int
driver(int, char **) {
  FTEST {
    execute<assign, single>(energy);
    EXPECT_EQ(test<check>(energy), 0);
  };
}

ftest_register_driver(driver);
