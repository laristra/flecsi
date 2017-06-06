/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/tuple_function.h"

// includes: other
#include <cinchtest.h>

// struct foo
struct foo {
   // operator(), 0 arguments
   decltype(auto) operator()(void) const
   {
      return "abc";
   }

   // operator(), 1 argument
   template<class A>
   decltype(auto) operator()(const A &a) const
   {
      return a+10;
   }

   // operator(), 2 arguments
   template<class A, class B>
   decltype(auto) operator()(const A &a, const B &b) const
   {
      return a+b;
   }

   // operator(), 3 arguments
   template<class A, class B, class C>
   decltype(auto) operator()(const A &a, const B &b, const C &c) const
   {
      return a+b+c;
   }
};



// =============================================================================
// Test various constructs in tuple_function.h
// =============================================================================

// TEST
TEST(tuple_function, all)
{
   std::tuple<                > zero;
   std::tuple<int             > one  (1                     );
   std::tuple<int,float       > two  (1, float(2)           );
   std::tuple<int,float,double> three(1, float(2), double(3));

   foo f;
   EXPECT_EQ(flecsi::utils::tuple_function(f, zero ), "abc");
   EXPECT_EQ(flecsi::utils::tuple_function(f, one  ), 11);
   EXPECT_EQ(flecsi::utils::tuple_function(f, two  ), 3);
   EXPECT_EQ(flecsi::utils::tuple_function(f, three), 6);

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
