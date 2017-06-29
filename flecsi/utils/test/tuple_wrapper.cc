/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/tuple_wrapper.h"

// includes: other
#include <cinchtest.h>
#include "boost/core/demangle.hpp"

// print_type
inline std::string typestr(const char *const name)
{
   return boost::core::demangle(name);
}



// =============================================================================
// Test various constructs in tuple_wrapper.h
// =============================================================================

// TEST
TEST(tuple_wrapper, all)
{
   // generic_tuple_t
   flecsi::utils::generic_tuple_t a;
   (void)a;

   // tuple_wrapper_
   flecsi::utils::tuple_wrapper_<> b;
   flecsi::utils::tuple_wrapper_<char> c('c');;
   flecsi::utils::tuple_wrapper_<int,double> d(1, 2.0);
   flecsi::utils::tuple_wrapper_<float,char,int> e(1.0f, 'x', 2);

   // tuple_wrapper_::tuple_t
   EXPECT_EQ(
      typestr(
         typeid(
            typename flecsi::utils::tuple_wrapper_<float,char,int>::tuple_t
         ).name()
      ),
     "std::tuple<float, char, int>"
   );

   // tuple_wrapper_::get
   EXPECT_EQ(c.get<0>(), 'c');
   EXPECT_EQ(d.get<0>(), 1);
   EXPECT_EQ(d.get<1>(), 2.0);
   EXPECT_EQ(e.get<0>(), 1.0f);
   EXPECT_EQ(e.get<1>(), 'x');
   EXPECT_EQ(e.get<2>(), 2);

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
