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

#include "flecsi/util/set_utils.hh"
#include "flecsi/util/set_intersection.hh"
#include "flecsi/util/unit.hh"

#include <iostream>

template<class T>
void
print_set(const char * const prefix, const std::set<T> & set) {
  UNIT_CAPTURE() << prefix << " == {";
  for(auto i = set.begin(); i != set.end(); ++i)
    UNIT_CAPTURE() << " " << *i;
  UNIT_CAPTURE() << " }" << std::endl;
}

template<class CONTAINER>
inline bool
intersects(const CONTAINER & one, const CONTAINER & two) {
  return flecsi::util::intersects(
    one.begin(), one.end(), two.begin(), two.end());
}

int
set_utils() {
  UNIT {
    using namespace flecsi::util;

    std::set<std::size_t> a = {1, 3, 5, 7, 10, 11};
    std::set<std::size_t> b = {2, 3, 6, 7, 10, 12};
    std::set<std::size_t> c = {0, 20};
    std::set<std::size_t> d = {0, 10, 20};
    std::set<std::size_t> e = {};

    print_set("a", a);
    print_set("b", b);
    print_set("c", c);
    print_set("d", d);
    print_set("e", e);
    UNIT_CAPTURE() << std::endl;

    print_set("intersection(a,a)", set_intersection(a, a));
    print_set("intersection(a,b)", set_intersection(a, b));
    print_set("intersection(a,c)", set_intersection(a, c));
    print_set("intersection(a,d)", set_intersection(a, d));
    print_set("intersection(a,e)", set_intersection(a, e));
    print_set("intersection(b,b)", set_intersection(a, b));
    print_set("intersection(b,c)", set_intersection(a, c));
    print_set("intersection(b,d)", set_intersection(a, d));
    print_set("intersection(b,e)", set_intersection(a, e));
    print_set("intersection(c,c)", set_intersection(a, c));
    print_set("intersection(c,d)", set_intersection(a, d));
    print_set("intersection(c,e)", set_intersection(a, e));
    print_set("intersection(d,d)", set_intersection(a, d));
    print_set("intersection(d,e)", set_intersection(a, e));
    print_set("intersection(e,e)", set_intersection(a, e));
    UNIT_CAPTURE() << std::endl;

    print_set("union(a,a)", set_union(a, a));
    print_set("union(a,b)", set_union(a, b));
    print_set("union(a,c)", set_union(a, c));
    print_set("union(a,d)", set_union(a, d));
    print_set("union(a,e)", set_union(a, e));
    print_set("union(b,b)", set_union(a, b));
    print_set("union(b,c)", set_union(a, c));
    print_set("union(b,d)", set_union(a, d));
    print_set("union(b,e)", set_union(a, e));
    print_set("union(c,c)", set_union(a, c));
    print_set("union(c,d)", set_union(a, d));
    print_set("union(c,e)", set_union(a, e));
    print_set("union(d,d)", set_union(a, d));
    print_set("union(d,e)", set_union(a, e));
    print_set("union(e,e)", set_union(a, e));
    UNIT_CAPTURE() << std::endl;

    print_set("difference(a,a)", set_difference(a, a));
    print_set("difference(a,b)", set_difference(a, b));
    print_set("difference(a,c)", set_difference(a, c));
    print_set("difference(a,d)", set_difference(a, d));
    print_set("difference(a,e)", set_difference(a, e));
    print_set("difference(b,b)", set_difference(a, b));
    print_set("difference(b,c)", set_difference(a, c));
    print_set("difference(b,d)", set_difference(a, d));
    print_set("difference(b,e)", set_difference(a, e));
    print_set("difference(c,c)", set_difference(a, c));
    print_set("difference(c,d)", set_difference(a, d));
    print_set("difference(c,e)", set_difference(a, e));
    print_set("difference(d,d)", set_difference(a, d));
    print_set("difference(d,e)", set_difference(a, e));
    print_set("difference(e,e)", set_difference(a, e));

    {
      std::vector<int> a = {1, 3, 5, 7, 10, 11};
      std::vector<int> b = {2, 4, 6, 8, 10, 12};
      std::vector<int> c = {0, 20};
      std::vector<int> d = {10, 20, 30};
      std::vector<int> e = {};
      std::vector<int> f = {};

      EXPECT_EQ(intersects(a, a), true);
      EXPECT_EQ(intersects(a, b), true);
      EXPECT_EQ(intersects(a, c), false);
      EXPECT_EQ(intersects(a, d), true);
      EXPECT_EQ(intersects(b, a), true);
      EXPECT_EQ(intersects(b, b), true);
      EXPECT_EQ(intersects(b, c), false);
      EXPECT_EQ(intersects(b, d), true);
      EXPECT_EQ(intersects(c, a), false);
      EXPECT_EQ(intersects(c, b), false);
      EXPECT_EQ(intersects(c, c), true);
      EXPECT_EQ(intersects(c, d), true);
      EXPECT_EQ(intersects(d, a), true);
      EXPECT_EQ(intersects(d, b), true);
      EXPECT_EQ(intersects(d, c), true);
      EXPECT_EQ(intersects(d, d), true);
      EXPECT_EQ(intersects(a, e), false);
      EXPECT_EQ(intersects(e, f), false);
    }

    // compare
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("set_utils.blessed"));
  };
} // set_utils

flecsi::unit::driver<set_utils> driver;
