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

#include <flecsi/utils/common.hh>
#include <flecsi/utils/ftest.hh>

struct MyClass {
  int operator()(float, double, long double) const {
    return 0;
  }

  void mem(char, int) {}
  void memc(char, int) const {}
  void memv(char, int) volatile {}
  void memcv(char, int) const volatile {}
};

inline float
MyFun(double, int, long) {
  return float(0);
}

int
common(int, char **) {
  FTEST {
    // *BITS #defines
    FTEST_CAPTURE() << FLECSI_ID_PBITS << std::endl;
    FTEST_CAPTURE() << FLECSI_ID_EBITS << std::endl;
    FTEST_CAPTURE() << FLECSI_ID_FBITS << std::endl;
    FTEST_CAPTURE() << FLECSI_ID_GBITS << std::endl;
    FTEST_CAPTURE() << std::endl;

    // types
    FTEST_CAPTURE() << FTEST_TTYPE(flecsi::util::id_t) << std::endl;
    FTEST_CAPTURE() << FTEST_TTYPE(FLECSI_COUNTER_TYPE) << std::endl;
    FTEST_CAPTURE() << FTEST_TTYPE(flecsi::util::counter_t) << std::endl;
    FTEST_CAPTURE() << std::endl;

    // square
    FTEST_CAPTURE() << flecsi::util::square(10) << std::endl;
    FTEST_CAPTURE() << flecsi::util::square(20.0) << std::endl;
    FTEST_CAPTURE() << std::endl;

    // ------------------------
    // Unique ID constructs
    // ------------------------

    // FLECSI_GENERATED_ID_MAX
    // We won't test the particular value, as it looks like the sort
    // of thing that might change over time
    EXPECT_TRUE(FLECSI_GENERATED_ID_MAX > 0);

    // unique_id_t
    struct unique_type_t {};
    auto & a = flecsi::util::unique_id<unique_type_t, int, 10>::instance();
    auto & b = flecsi::util::unique_id<unique_type_t, int, 10>::instance();
    EXPECT_EQ(&a, &b); // because type is a singleton

    auto & c = flecsi::util::unique_id<unique_type_t, int>::instance();
    auto & d = flecsi::util::unique_id<unique_type_t, int>::instance();
    EXPECT_EQ(&c, &d); // singleton again
    EXPECT_NE(
      (void *)&c, (void *)&a); // != (different template specializations)

    FTEST_CAPTURE() << a.next() << std::endl;
    FTEST_CAPTURE() << a.next() << std::endl;
    FTEST_CAPTURE() << a.next() << std::endl;
    FTEST_CAPTURE() << std::endl;

    // unique_name
    // Just exercise; return value generally changes between runs
    const int i = 2;
    const float f = float(3.14);
    EXPECT_NE(flecsi::util::unique_name(&i), "");
    EXPECT_NE(flecsi::util::unique_name(&i), "");
    EXPECT_NE(flecsi::util::unique_name(&f), "");
    EXPECT_NE(flecsi::util::unique_name(&f), "");

    // ------------------------
    // Compare
    // ------------------------

#ifdef __GNUG__
#ifdef __PPC64__
    EXPECT_TRUE(FTEST_EQUAL_BLESSED("common.blessed.ppc"));
#else
    EXPECT_TRUE(FTEST_EQUAL_BLESSED("common.blessed.gnug"));
#endif
#elif defined(_MSC_VER)
    EXPECT_TRUE(FTEST_EQUAL_BLESSED("common.blessed.msvc"));
#else
    EXPECT_TRUE(FTEST_EQUAL_BLESSED("common.blessed"));
#endif
  };
}

ftest_register_driver(common);
