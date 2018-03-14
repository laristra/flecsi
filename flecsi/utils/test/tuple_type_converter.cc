/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/tuple_type_converter.h>
#include <flecsi/utils/common.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// print_type
inline void
print_type(const char * const name) {
#ifdef __GNUG__
  CINCH_CAPTURE() << flecsi::utils::demangle(name) << std::endl;
#else
  // Skip name printing; is unpredictable in this case
#endif
}

// a base class
class base {};

// a derived class
class derived : public base {};

// a further derived class
class further : public derived {};

// a class
class thing {};

// =============================================================================
// Test various constructs in tuple_type_converter.h
// =============================================================================

// TEST
TEST(tuple_type_converter, all) {
  // convert_tuple_type_
  print_type(
      typeid(typename flecsi::utils::convert_tuple_type_<char>::type).name());
  print_type(
      typeid(typename flecsi::utils::convert_tuple_type_<const char>::type)
          .name());
  CINCH_CAPTURE() << std::endl;

  // convert_tuple_type
  print_type(
      typeid(typename flecsi::utils::convert_tuple_type<std::tuple<>>::type)
          .name());
  print_type(
      typeid(typename flecsi::utils::convert_tuple_type<std::tuple<char>>::type)
          .name());
  print_type(typeid(typename flecsi::utils::convert_tuple_type<
                        std::tuple<int, double>>::type)
                 .name());
  CINCH_CAPTURE() << std::endl;

  // base_convert_tuple_type_
  print_type(typeid(typename flecsi::utils::base_convert_tuple_type_<
                        char, int, true>::type)
                 .name());
  print_type(typeid(typename flecsi::utils::base_convert_tuple_type_<
                        char, int, false>::type)
                 .name());
  CINCH_CAPTURE() << std::endl;

  // base_convert_tuple_type
  print_type(typeid(typename flecsi::utils::base_convert_tuple_type<
                        base, double, std::tuple<>>::type)
                 .name());

  print_type(
      typeid(typename flecsi::utils::base_convert_tuple_type<
                 base,   // use this by default...
                 double, // but use this where base is base of tuple element...
                 std::tuple<
                     int,
                     base,    // base is considered to be base
                     derived, // base is base
                     char, thing,
                     further // base is base
                     >>::type)
          .name());

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("tuple_type_converter.blessed.gnug"));
#endif
} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
