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
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// system includes
#include <cinchtest.h>
#include <array>

// user includes
#include "flecsi/utils/any.h"
#include "flecsi/partition/index_partition.h"

#include <array>

using flecsi::utils::any_t;
using flecsi::dmp::index_partition_t;
using std::cout;
using std::endl;

//=============================================================================
//! \brief Test the "zip-like" iterator.
//=============================================================================

///
/// some temporary class
///
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

///
/// test body
///
TEST(any, simple) {

   typedef std::vector<any_t> storage_t;
   typedef storage_t::const_iterator storage_iter;
   storage_t storage;

   double A;
   std::array<int,5> B;
   index_partition_t ip;

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

    //assert(typeid(new_ptr->type())==typeid(index_partition_t));
  //double* a=flecsi::utils::any_cast<double>(new_ptr);

  ASSERT_EQ( 3, storage.size() )  << "iterator count mismatch";

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
