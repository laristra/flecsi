/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/common.h>
#include <flecsi/utils/test/print_type.h>

// includes: other
#include <cinchtest.h>

// =============================================================================
// For testing flecsi::utils::function_traits_u
// =============================================================================

// a class...
struct MyClass {
  int operator()(float, double, long double) const {
    return 0;
  }

  void mem(char, int) {}
  void memc(char, int) const {}
  void memv(char, int) volatile {}
  void memcv(char, int) const volatile {}
};

// a function...
inline float
MyFun(double, int, long) {
  return float(0);
}

// =============================================================================
// Test various constructs in flecsi::utils::common.*
// =============================================================================

TEST(common, all) {
  // *BITS #defines
  CINCH_CAPTURE() << FLECSI_ID_PBITS << std::endl;
  CINCH_CAPTURE() << FLECSI_ID_EBITS << std::endl;
  CINCH_CAPTURE() << FLECSI_ID_FBITS << std::endl;
  CINCH_CAPTURE() << FLECSI_ID_GBITS << std::endl;
  CINCH_CAPTURE() << std::endl;

  // types
  print_type<flecsi::utils::id_t>();
  print_type<FLECSI_COUNTER_TYPE>();
  print_type<flecsi::utils::counter_t>();
  CINCH_CAPTURE() << std::endl;

  // square
  CINCH_CAPTURE() << flecsi::utils::square(10) << std::endl;
  CINCH_CAPTURE() << flecsi::utils::square(20.0) << std::endl;
  CINCH_CAPTURE() << std::endl;

  // ------------------------
  // Unique ID constructs
  // ------------------------

  // FLECSI_GENERATED_ID_MAX
  // We won't test the particular value, as it looks like the sort
  // of thing that might change over time
  EXPECT_TRUE(FLECSI_GENERATED_ID_MAX > 0);

  // unique_id_t
  auto & a = flecsi::utils::unique_id_t<int, 10>::instance();
  auto & b = flecsi::utils::unique_id_t<int, 10>::instance();
  EXPECT_EQ(&a, &b); // because type is a singleton

  auto & c = flecsi::utils::unique_id_t<int>::instance();
  auto & d = flecsi::utils::unique_id_t<int>::instance();
  EXPECT_EQ(&c, &d); // singleton again
  EXPECT_NE((void *)&c, (void *)&a); // != (different template specializations)

  CINCH_CAPTURE() << a.next() << std::endl;
  CINCH_CAPTURE() << a.next() << std::endl;
  CINCH_CAPTURE() << a.next() << std::endl;
  CINCH_CAPTURE() << std::endl;

  // unique_name
  // Just exercise; return value generally changes between runs
  const int i = 2;
  const float f = float(3.14);
  EXPECT_NE(flecsi::utils::unique_name(&i), "");
  EXPECT_NE(flecsi::utils::unique_name(&i), "");
  EXPECT_NE(flecsi::utils::unique_name(&f), "");
  EXPECT_NE(flecsi::utils::unique_name(&f), "");

  // ------------------------
  // Compare
  // ------------------------

#ifdef __GNUG__
#ifdef __PPC64__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("common.blessed.ppc"));
#else
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("common.blessed.gnug"));
#endif
#elif defined(_MSC_VER)
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("common.blessed.msvc"));
#else
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("common.blessed"));
#endif
} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
