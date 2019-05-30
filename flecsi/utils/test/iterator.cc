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

#include <flecsi/utils/common.h>
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/iterator.h>

#include <array>
#include <iostream>
#include <vector>

/*
   Test various aspects of flecsi::utils::iterator
 */
int
iterator(int argc, char ** argv) {

  FTEST();

  // some containers
  std::vector<int> veci{1, 2, 3, 5, 7, 11, 13, 17}; // we'll start at [1] :-)
  std::array<double, 5> vecd{{1.234, 5.678, 3.1416, 2.7183, 1.414}}; // at [2]

  // test: types (container_t and type_t)
  using vector_int_container_t =
    flecsi::utils::iterator<std::vector<int>, int>::container_t;
  FTEST_CAPTURE() << FTEST_TTYPE(vector_int_container_t) << std::endl;
  using vector_int_type_t =
    flecsi::utils::iterator<std::vector<int>, int>::type_t;
  FTEST_CAPTURE() << FTEST_TTYPE(vector_int_type_t) << std::endl;
  using array_double_container_t =
    flecsi::utils::iterator<std::array<double, 5>, double>::container_t;
  FTEST_CAPTURE() << FTEST_TTYPE(array_double_container_t) << std::endl;
  using array_double_type_t =
    flecsi::utils::iterator<std::array<double, 5>, double>::type_t;
  FTEST_CAPTURE() << FTEST_TTYPE(array_double_type_t) << std::endl;

  // test: constructor from container and index
  flecsi::utils::iterator<std::vector<int>, int> i(veci, 1);
  flecsi::utils::iterator<std::array<double, 5>, double> d(vecd, 2);

  // test: destructor
  flecsi::utils::iterator<std::vector<int>, int> * iptr =
    new flecsi::utils::iterator<std::vector<int>, int>(veci, 1);
  delete iptr;

  // test: copy constructor
  flecsi::utils::iterator<std::vector<int>, int> i2 = i;
  flecsi::utils::iterator<std::array<double, 5>, double> d2 = d;

  // test: assignment operator
  flecsi::utils::iterator<std::vector<int>, int> i3(veci, 1);
  i3 = i;
  flecsi::utils::iterator<std::array<double, 5>, double> d3(vecd, 2);
  d3 = d;

  // test: pre-increment
  ++i;
  FTEST_CAPTURE() << "The second prime is: " << *i << std::endl; // should be 3
  ++(++i); // works, because return is &
  FTEST_CAPTURE() << "The fourth prime is: " << *i << std::endl; // should be 7

  // test: dereference
  EXPECT_EQ(*i, 7);
  EXPECT_EQ(*d, 3.1416);

  // test: equivalence
  flecsi::utils::iterator<std::vector<int>, int> i4(veci, 1);
  i4 = i;
  EXPECT_TRUE(i4 == i);

  // test: non-equivalence
  EXPECT_TRUE(i2 != i); // because there were some i++s after constructing i2

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("iterator.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("iterator.blessed.msvc"));
#else
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("iterator.blessed"));
#endif

  return 0;
} // TEST

ftest_register_driver(iterator);
