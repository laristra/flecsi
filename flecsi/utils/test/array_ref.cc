/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// includes: flecsi
#include "flecsi/utils/array_ref.h"

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>
#include "boost/core/demangle.hpp"



// print_type
inline void print_type(const char *const name)
{
   CINCH_CAPTURE() << boost::core::demangle(name) << std::endl;
}

// print_refc
// Prints an array_ref<char>, as a character string
inline void print_refc(const flecsi::utils::array_ref<char> &arr)
{
   for (auto c = arr.begin();  c != arr.end();  ++c)
      CINCH_CAPTURE() << *c;
   CINCH_CAPTURE() << std::endl;
}

using flecsi::utils::array_ref;



//=============================================================================
//! \brief Test various constructs in array_ref.h
//=============================================================================

TEST(array_ref, all) {

   using refd = array_ref<double>;
   using refc = array_ref<char>;
   using reff = array_ref<float>;
   using refi = array_ref<int>;

   CINCH_CAPTURE() << std::endl;


   // ------------------------
   // types
   // ------------------------

   print_type(typeid(refd::value_type            ).name());
   print_type(typeid(refd::pointer               ).name());
   print_type(typeid(refd::reference             ).name());
   print_type(typeid(refd::const_reference       ).name());
   print_type(typeid(refd::const_iterator        ).name());
   print_type(typeid(refd::iterator              ).name());
   print_type(typeid(refd::const_reverse_iterator).name());
   print_type(typeid(refd::reverse_iterator      ).name());
   print_type(typeid(refd::size_type             ).name());
   print_type(typeid(refd::difference_type       ).name());
   CINCH_CAPTURE() << std::endl;


   // ------------------------
   // constructor/assignment
   // ------------------------

   // default constructor
   refd a;
   EXPECT_EQ(a.begin(), nullptr);
   EXPECT_EQ(a.end  (), nullptr);

   // constructor from pointer and length
   const std::size_t length = 5;
   double plain_array[length];
   refd b(plain_array,length);
   EXPECT_EQ(b.begin(), &plain_array[0]);
   EXPECT_EQ(b.end  (), &plain_array[0]+length);

   // copy constructor
   refd c = b;
   EXPECT_EQ(c.begin(), &plain_array[0]);
   EXPECT_EQ(c.end  (), &plain_array[0]+length);

   // copy assignment
   c = c = b;
   EXPECT_EQ(c.begin(), &plain_array[0]);
   EXPECT_EQ(c.end  (), &plain_array[0]+length);


   // ------------------------
   // conversion
   // ------------------------

   // conversion from std::vector
   const std::vector<double> vec = { 3.14, 2.18, 2.72 };
   refd d = vec;
   EXPECT_EQ(d.begin(), &vec[0]);
   EXPECT_EQ(d.end  (), &vec[0]+vec.size());

   // conversion from std::string
   const std::string str = "hello";
   const refc e = str;
   EXPECT_EQ(e.begin(), &str[0]);
   EXPECT_EQ(e.end  (), &str[0]+str.size());

   // conversion from C-style array
   const std::size_t arrlen = 9;
   const float arr[arrlen] = { 1, 9, 2, 8, 3, 7, 4, 6, 5 };
   const reff f = arr;
   EXPECT_EQ(f.begin(), &arr[0]);
   EXPECT_EQ(f.end  (), &arr[0]+arrlen);

   // conversion from std::array
   const std::array<int,2> ints = {{ 10, 20 }};
   const refi g = ints;
   EXPECT_EQ(g.begin(), &ints[0]);
   EXPECT_EQ(g.end  (), &ints[0]+ints.size());


   // ------------------------
   // substr
   // ------------------------

   // ......................012345678
   const std::string abc = "abcdefghi";

   print_refc(refc(abc));
   CINCH_CAPTURE() << std::endl;

   print_refc(refc(abc).substr(0));
   print_refc(refc(abc).substr(1));
   print_refc(refc(abc).substr(2));
   print_refc(refc(abc).substr(7));
   print_refc(refc(abc).substr(8));
   print_refc(refc(abc).substr(9));    // blank
   print_refc(refc(abc).substr(100));  // blank
   CINCH_CAPTURE() << std::endl;

   print_refc(refc(abc).substr(0,2));
   print_refc(refc(abc).substr(1,4));
   print_refc(refc(abc).substr(2,3));
   print_refc(refc(abc).substr(7,5));
   print_refc(refc(abc).substr(8,1));
   print_refc(refc(abc).substr(9,1));    // blank
   print_refc(refc(abc).substr(100,1));  // blank
   CINCH_CAPTURE() << std::endl;


   // ------------------------
   // iterators
   // ------------------------

   for (auto it = abc.begin();  it != abc.end();  ++it)
      CINCH_CAPTURE() << *it;
   CINCH_CAPTURE() << std::endl;
   for (auto it = abc.cbegin();  it != abc.cend();  ++it)
      CINCH_CAPTURE() << *it;
   CINCH_CAPTURE() << std::endl;
   for (auto it = abc.rbegin();  it != abc.rend();  ++it)
      CINCH_CAPTURE() << *it;
   CINCH_CAPTURE() << std::endl;
   for (auto it = abc.crbegin();  it != abc.crend();  ++it)
      CINCH_CAPTURE() << *it;
   CINCH_CAPTURE() << std::endl;


   // ------------------------
   // capacity
   // ------------------------

   const refd cap0;
   const std::vector<double> vec3 = { 3.14, 2.18, 2.72 };
   const refd cap3 = vec3;

   CINCH_CAPTURE() <<  cap0.size() << std::endl;
   CINCH_CAPTURE() << (cap0.empty() ? "true" : "false") << std::endl; // empty
   CINCH_CAPTURE() <<  cap3.size() << std::endl;
   CINCH_CAPTURE() << (cap3.empty() ? "true" : "false") << std::endl; // not
   CINCH_CAPTURE() << std::endl;

   // max_size() is machine-dependent, so I won't put it into a comparison
   // file. Let's basically just be sure it's callable and reasonable...
   EXPECT_TRUE(0 < cap0.max_size());
   EXPECT_TRUE(0 < cap3.max_size());


   // ------------------------
   // element access
   // ------------------------

   // []
   CINCH_CAPTURE() << cap3[0] << std::endl;
   CINCH_CAPTURE() << cap3[1] << std::endl;
   CINCH_CAPTURE() << cap3[2] << std::endl;
   CINCH_CAPTURE() << std::endl;

   // at()
   CINCH_CAPTURE() << cap3.at(0) << std::endl;
   CINCH_CAPTURE() << cap3.at(1) << std::endl;
   CINCH_CAPTURE() << cap3.at(2) << std::endl;
   CINCH_CAPTURE() << std::endl;

   // test at() exception
   try {
      CINCH_CAPTURE() << cap3.at(3) << std::endl;
   }
   catch (...)
   {
      CINCH_CAPTURE() << "Caught an (intentionally generated!) test exception";
   }
   CINCH_CAPTURE() << std::endl;

   // front(), back()
   CINCH_CAPTURE() << cap3.front() << std::endl;
   CINCH_CAPTURE() << cap3.back () << std::endl;
   CINCH_CAPTURE() << std::endl;

   // data()
   EXPECT_NE(cap3.data(), nullptr);


   // ------------------------
   // outgoing conversion
   // ------------------------

   // to std::vector
   EXPECT_EQ(vec3, std::vector<double>(cap3));
   EXPECT_EQ(vec3, cap3.vec());

   // to std::string
   EXPECT_EQ(abc, std::string(refc(abc)));
   EXPECT_EQ(abc, refc(abc).str());


   // ------------------------
   // mutators
   // ------------------------

   {
      // clear()
      const std::vector<double> vec = { 3.14, 2.18, 2.72 };
      refd arr = vec;
      EXPECT_NE(arr.begin(), nullptr);
      EXPECT_NE(arr.size(), 0);
      arr.clear();
      EXPECT_EQ(arr.begin(), nullptr);
      EXPECT_EQ(arr.size(), 0);

      // remove_*()
      int iarray[9] = { 1, 9, 2, 8, 3, 7, 4, 6, 5 };
      refi f = iarray;
      f.remove_prefix(2);
      f.remove_suffix(1);
      CINCH_CAPTURE() << f.front() << std::endl;
      CINCH_CAPTURE() << f.back () << std::endl;
      CINCH_CAPTURE() << std::endl;

      f.remove_prefix(3);
      f.remove_suffix(2);
      CINCH_CAPTURE() << f.front() << std::endl;
      CINCH_CAPTURE() << f.back () << std::endl;
      CINCH_CAPTURE() << std::endl;

      f.remove_prefix(1);  // ==> nothing left!
      EXPECT_EQ(f.size(), 0);
      for (auto it = f.begin();  it != f.end();  ++it)
         assert(false);

      // pop_*()
      double darray[3] = { 1.23, 4.56, 7.89 };
      refd d = darray;
      d.pop_back();
      d.pop_front();
      CINCH_CAPTURE() << d.front() << std::endl;
      CINCH_CAPTURE() << d.back () << std::endl;
      CINCH_CAPTURE() << std::endl;
   }


   // ------------------------
   // make_array_ref
   // ------------------------

   {
      // from * and length
      const std::size_t length = 9;
      int ints[length] = { 1, 9, 2, 8, 3, 7, 4, 6, 5 };
      refi a = flecsi::utils::make_array_ref(&ints[0], length);
      CINCH_CAPTURE() << a.front() << '\n';
      CINCH_CAPTURE() << a.back () << '\n' << std::endl;

      // from T [n]
      refi b = flecsi::utils::make_array_ref(ints);
      CINCH_CAPTURE() << b.front() << '\n';
      CINCH_CAPTURE() << b.back () << '\n' << std::endl;

      // from std::vector
      std::vector<int> ivec(10,1);  // 10 1s
      refi c = flecsi::utils::make_array_ref(ivec);
      CINCH_CAPTURE() << c.front() << '\n';
      CINCH_CAPTURE() << c.back () << '\n' << std::endl;

      // from std::array
      const std::array<int,2> iarr = {{ 10, 20 }};
      refi d = flecsi::utils::make_array_ref(iarr);
      CINCH_CAPTURE() << d.front() << '\n';
      CINCH_CAPTURE() << d.back () << '\n';
   }


   // ------------------------
   // compare
   // ------------------------

   EXPECT_TRUE(CINCH_EQUAL_BLESSED("array_ref.blessed"));

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
