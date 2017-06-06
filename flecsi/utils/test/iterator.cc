/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/iterator.h"

// includes: C++
#include <array>
#include <iostream>
#include <vector>

// includes: other
#include <cinchtest.h>
#include "boost/core/demangle.hpp"

// print_type
inline void print_type(const char *const name)
{
   CINCH_CAPTURE() << boost::core::demangle(name) << std::endl;
}



// =============================================================================
// Test various aspects of flecsi::utils::iterator
// =============================================================================

TEST(iterator, all)
{
   // some containers
   std::vector<int> veci { 1,2,3,5,7,11,13,17 };  // we'll start at [1] :-)
   std::array<double,5> vecd {{ 1.234,5.678,3.1416,2.7183,1.414 }};  // at [2]

   // test: types (container_t and type_t)
   print_type(typeid(flecsi::utils::
      iterator<std::vector<int>,int>::container_t).name());
   print_type(typeid(flecsi::utils::
      iterator<std::vector<int>,int>::type_t).name());
   print_type(typeid(flecsi::utils::
      iterator<std::array<double,5>,double>::container_t).name());
   print_type(typeid(flecsi::utils::
      iterator<std::array<double,5>,double>::type_t).name());

   // test: constructor from container and index
   flecsi::utils::iterator<std::vector<int>,int> i(veci,1);
   flecsi::utils::iterator<std::array<double,5>,double> d(vecd,2);

   // test: destructor
   flecsi::utils::iterator<std::vector<int>,int> *iptr =
      new flecsi::utils::iterator<std::vector<int>,int>(veci,1);
   delete iptr;

   // test: copy constructor
   flecsi::utils::iterator<std::vector<int>,int> i2 = i;
   flecsi::utils::iterator<std::array<double,5>,double> d2 = d;

   // test: assignment operator
   flecsi::utils::iterator<std::vector<int>,int> i3(veci,1);
   i3 = i;
   flecsi::utils::iterator<std::array<double,5>,double> d3(vecd,2);
   d3 = d;

   // test: pre-increment
   ++i;
   CINCH_CAPTURE() << "The second prime is: " << *i << std::endl; // should be 3
   ++(++i);  // works, because return is &
   CINCH_CAPTURE() << "The fourth prime is: " << *i << std::endl; // should be 7

   // test: dereference
   EXPECT_EQ(*i, 7);
   EXPECT_EQ(*d, 3.1416);

   // test: equivalence
   flecsi::utils::iterator<std::vector<int>,int> i4(veci,1);
   i4 = i;
   EXPECT_TRUE(i4 == i);

   // test: non-equivalence
   EXPECT_TRUE(i2 != i);  // because there were some i++s after constructing i2

   // compare
   EXPECT_TRUE(CINCH_EQUAL_BLESSED("iterator.blessed"));

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
