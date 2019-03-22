/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/ftest.h>
#include <flecsi/utils/set_utils.h>

#include <iostream>

template<class T>
void
print_set(const char * const prefix, const std::set<T> & set) {
  FTEST_CAPTURE() << prefix << " == {";
  for(auto i = set.begin(); i != set.end(); ++i)
    FTEST_CAPTURE() << " " << *i;
  FTEST_CAPTURE() << " }" << std::endl;
}

int
set_utils(int argc, char ** argv) {

  FTEST();

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
  FTEST_CAPTURE() << std::endl;

  print_set("intersection(a,a)", flecsi::utils::set_intersection(a, a));
  print_set("intersection(a,b)", flecsi::utils::set_intersection(a, b));
  print_set("intersection(a,c)", flecsi::utils::set_intersection(a, c));
  print_set("intersection(a,d)", flecsi::utils::set_intersection(a, d));
  print_set("intersection(a,e)", flecsi::utils::set_intersection(a, e));
  print_set("intersection(b,b)", flecsi::utils::set_intersection(a, b));
  print_set("intersection(b,c)", flecsi::utils::set_intersection(a, c));
  print_set("intersection(b,d)", flecsi::utils::set_intersection(a, d));
  print_set("intersection(b,e)", flecsi::utils::set_intersection(a, e));
  print_set("intersection(c,c)", flecsi::utils::set_intersection(a, c));
  print_set("intersection(c,d)", flecsi::utils::set_intersection(a, d));
  print_set("intersection(c,e)", flecsi::utils::set_intersection(a, e));
  print_set("intersection(d,d)", flecsi::utils::set_intersection(a, d));
  print_set("intersection(d,e)", flecsi::utils::set_intersection(a, e));
  print_set("intersection(e,e)", flecsi::utils::set_intersection(a, e));
  FTEST_CAPTURE() << std::endl;

  print_set("union(a,a)", flecsi::utils::set_union(a, a));
  print_set("union(a,b)", flecsi::utils::set_union(a, b));
  print_set("union(a,c)", flecsi::utils::set_union(a, c));
  print_set("union(a,d)", flecsi::utils::set_union(a, d));
  print_set("union(a,e)", flecsi::utils::set_union(a, e));
  print_set("union(b,b)", flecsi::utils::set_union(a, b));
  print_set("union(b,c)", flecsi::utils::set_union(a, c));
  print_set("union(b,d)", flecsi::utils::set_union(a, d));
  print_set("union(b,e)", flecsi::utils::set_union(a, e));
  print_set("union(c,c)", flecsi::utils::set_union(a, c));
  print_set("union(c,d)", flecsi::utils::set_union(a, d));
  print_set("union(c,e)", flecsi::utils::set_union(a, e));
  print_set("union(d,d)", flecsi::utils::set_union(a, d));
  print_set("union(d,e)", flecsi::utils::set_union(a, e));
  print_set("union(e,e)", flecsi::utils::set_union(a, e));
  FTEST_CAPTURE() << std::endl;

  print_set("difference(a,a)", flecsi::utils::set_difference(a, a));
  print_set("difference(a,b)", flecsi::utils::set_difference(a, b));
  print_set("difference(a,c)", flecsi::utils::set_difference(a, c));
  print_set("difference(a,d)", flecsi::utils::set_difference(a, d));
  print_set("difference(a,e)", flecsi::utils::set_difference(a, e));
  print_set("difference(b,b)", flecsi::utils::set_difference(a, b));
  print_set("difference(b,c)", flecsi::utils::set_difference(a, c));
  print_set("difference(b,d)", flecsi::utils::set_difference(a, d));
  print_set("difference(b,e)", flecsi::utils::set_difference(a, e));
  print_set("difference(c,c)", flecsi::utils::set_difference(a, c));
  print_set("difference(c,d)", flecsi::utils::set_difference(a, d));
  print_set("difference(c,e)", flecsi::utils::set_difference(a, e));
  print_set("difference(d,d)", flecsi::utils::set_difference(a, d));
  print_set("difference(d,e)", flecsi::utils::set_difference(a, e));
  print_set("difference(e,e)", flecsi::utils::set_difference(a, e));

  // compare
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("set_utils.blessed"));

  return 0;
}

ftest_register_test(set_utils);
