/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// system includes
#include <cinchtest.h>
#include <array>

// user includes
#include "flecsi/utils/any.h"
#include "flecsi/coloring/index_coloring.h"

// boost includes
#include "boost/core/demangle.hpp"

using flecsi::utils::any_t;
using flecsi::coloring::index_coloring_t;
using std::cout;
using std::endl;



//=============================================================================
//! \brief Test the "zip-like" iterator.
//=============================================================================

//!
//! some temporary class
//!
class temp_class_t
{
  int value;
public:
  temp_class_t(int value) : value(value){}

  void show()
  {
    cout << "Hoge : " << value << endl;
  }
};

//!
//! test body
//!
TEST(any, simple) {

   typedef std::vector<any_t> storage_t;
   typedef storage_t::const_iterator storage_iter;
   storage_t storage;

   double A;
   std::array<int,5> B;
   index_coloring_t ip;

   A=3.14;
   B[0]=1;
   B[1]=2;

   storage.push_back(any_t(A));
   storage.push_back(any_t(B));
   storage.push_back(any_t(ip));

   int count =0;
   for (storage_iter i=storage.begin(); i!=storage.end(); ++i)
   {
      if (typeid(double) == i->get_type())
         assert(count == 0);
    count++;
   }

   double Ad= storage[0];
   std::cout <<"storage[0] = "<< Ad<<std::endl;

   any_t val;

   val = 10;
   int i = val;
   cout << i << endl;

   val = 3.14;
   cout << flecsi::utils::any_cast<double>(val) << endl;

   val = temp_class_t(123);
   temp_class_t h = val;
   h.show();

   val = std::string("Hello World!");
   cout << flecsi::utils::any_cast<std::string>(val) << endl;

//  std::shared_ptr<any_t> original_ptr = std::make_shared<any_t>(ip);

  //void* void_ptr = reinterpret_cast<void*>(&A);

  //any_t* new_ptr = reinterpret_cast<any_t*>(void_ptr);

    //assert(typeid(new_ptr->type())==typeid(index_coloring_t));
  //double* a=flecsi::utils::any_cast<double>(new_ptr);

  ASSERT_EQ( 3, storage.size() )  << "iterator count mismatch";

} // TEST



//=============================================================================
//! \brief Exercise everything in the any_t class, and any_cast() as well
//=============================================================================

// print_type
inline void print_type(const char *const name)
{
   CINCH_CAPTURE() << boost::core::demangle(name) << std::endl;
}

// some_class
template<class T>
class some_class
{
   T value;
public:
   inline explicit some_class(const T &value) : value(value) { }
   inline void print(void) const
   {
      CINCH_CAPTURE() << "value == " << value << std::endl;
   }
};



// TEST
TEST(any, all)
{
   // test: default constructor
   const flecsi::utils::any_t a;
   EXPECT_EQ(a.get_type(), typeid(void));  // via nullptr

   // test: constructor from T
   const flecsi::utils::any_t b = 3.14;
   EXPECT_EQ(b.get_type(), typeid(double));

   // test: destructor
   const flecsi::utils::any_t *const c = new flecsi::utils::any_t(4321);
   EXPECT_EQ(c->get_type(), typeid(int));
   delete c;

   // test: copy constructor
   const flecsi::utils::any_t a2 = a;
   const flecsi::utils::any_t b2 = b;
   EXPECT_EQ(a2.get_type(), typeid(void));
   EXPECT_EQ(b2.get_type(), typeid(double));

   // test: copy assignment
   flecsi::utils::any_t d, e;
   d = a;  EXPECT_EQ(d.get_type(), typeid(void));
   d = b;  EXPECT_EQ(d.get_type(), typeid(double));
   e = d = flecsi::utils::any_t(321);  // string 'em together
   EXPECT_EQ(d.get_type(), typeid(int));
   EXPECT_EQ(e.get_type(), typeid(int));

   // test: cast to T
   CINCH_CAPTURE() << double(b) << std::endl;
   CINCH_CAPTURE() << int   (e) << std::endl;
   //CINCH_CAPTURE() << char(e) << std::endl;  // wrong; would be a bad cast!

   // test: get_type()
   const flecsi::utils::any_t f = 1;
   const flecsi::utils::any_t g = 1.0;
   const flecsi::utils::any_t h = std::vector<int>{};
   const flecsi::utils::any_t i = std::vector<double>{};
   const flecsi::utils::any_t j = 'x';
   print_type(f.get_type().name());
   print_type(g.get_type().name());
   print_type(h.get_type().name());
   print_type(i.get_type().name());
   print_type(j.get_type().name());

   // test: any_cast()
   const flecsi::utils::any_t anyint = 123;
   const flecsi::utils::any_t anydbl = 4.5;
   CINCH_CAPTURE() << flecsi::utils::any_cast<int   >(anyint) << std::endl;
   CINCH_CAPTURE() << flecsi::utils::any_cast<double>(anydbl) << std::endl;
   CINCH_CAPTURE() << std::endl;

   // ------------------------
   // Some more-complex tests
   // ------------------------

   // fill vector<any_t> with any_t values from various types
   std::vector<flecsi::utils::any_t> vec;
   vec.push_back(flecsi::utils::any_t(1234));
   vec.push_back(flecsi::utils::any_t(5.67));
   vec.push_back(flecsi::utils::any_t(some_class<int>(8)));
   vec.push_back(flecsi::utils::any_t(some_class<double>(9.01)));
   vec.push_back(flecsi::utils::any_t(std::string("abc")));

   // ensure that the types are as expected
   EXPECT_EQ(vec[0].get_type(), typeid(int   ));
   EXPECT_EQ(vec[1].get_type(), typeid(double));
   EXPECT_EQ(vec[2].get_type(), typeid(some_class<int   >));
   EXPECT_EQ(vec[3].get_type(), typeid(some_class<double>));
   EXPECT_EQ(vec[4].get_type(), typeid(std::string));

   // print the wrapped values, via the any_t conversion operator
   CINCH_CAPTURE() << int   (vec[0]) << std::endl;
   CINCH_CAPTURE() << double(vec[1]) << std::endl;
   vec[2].operator some_class<int   >().print();
   vec[3].operator some_class<double>().print();
   CINCH_CAPTURE() << vec[4].operator std::string() << std::endl;

   // print the wrapped values, via any_cast
   CINCH_CAPTURE() << flecsi::utils::any_cast<int   >(vec[0]) << std::endl;
   CINCH_CAPTURE() << flecsi::utils::any_cast<double>(vec[1]) << std::endl;
   flecsi::utils::any_cast<some_class<int   >>(vec[2]).print();
   flecsi::utils::any_cast<some_class<double>>(vec[3]).print();
   CINCH_CAPTURE() << flecsi::utils::any_cast<std::string>(vec[4]) << std::endl;

   // compare
   EXPECT_TRUE(CINCH_EQUAL_BLESSED("any.blessed"));

} // TEST



/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
