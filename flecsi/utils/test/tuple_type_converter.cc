/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/common.h>
#include <flecsi/utils/test/print_type.h>
#include <flecsi/utils/tuple_type_converter.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// a base class
class base
{
};

// a derived class
class derived : public base
{
};

// a further derived class
class further : public derived
{
};

// a class
class thing
{
};

// =============================================================================
// Test various constructs in tuple_type_converter.h
// =============================================================================

// TEST
TEST(tuple_type_converter, all) {
  using namespace flecsi::utils;
  static_assert(std::is_same_v<convert_tuple_t<std::tuple<>,std::remove_reference_t>,std::tuple<>>);
  static_assert(std::is_same_v<convert_tuple_t<std::tuple<int,int&>,std::remove_reference_t>,std::tuple<int,int>>);

  // base_convert_tuple_type_
  print_type<
    typename flecsi::utils::base_convert_tuple_type_<char, int, true>::type>();
  print_type<
    typename flecsi::utils::base_convert_tuple_type_<char, int, false>::type>();
  CINCH_CAPTURE() << std::endl;

  // base_convert_tuple_type
  print_type<typename flecsi::utils::base_convert_tuple_type<base, double,
    std::tuple<>>::type>();

  print_type<typename flecsi::utils::base_convert_tuple_type<base, // use this
                                                                   // by
                                                                   // default...
    double, // but use this where base is base of tuple element...
    std::tuple<int,
      base, // base is considered to be base
      derived, // base is base
      char, thing,
      further // base is base
      >>::type>();

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("tuple_type_converter.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("tuple_type_converter.blessed.msvc"));
#endif
} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
