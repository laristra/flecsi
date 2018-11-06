/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <flecsi/utils/common.h>
#include <flecsi/utils/test/print_type.h>

struct type_t {
  int operator()(float, double, long double) const {
    return 0;
  }

  void mem(char, int) {}
  void memc(char, int) const {}
  void memv(char, int) volatile {}
  void memcv(char, int) const volatile {}
};

inline float
test_function(double, int, long) {
  return float(0);
}

TEST(common, all) {

  using flecsi::utils::function_traits_u;

  // general
  print_type<function_traits_u<type_t>::return_type>();
  print_type<function_traits_u<type_t>::arguments_type>();
  CINCH_CAPTURE() << std::endl;

  // f(...)
  // &f(...)
  print_type<function_traits_u<decltype(test_function)>::return_type>();
  print_type<function_traits_u<decltype(test_function)>::arguments_type>();
  print_type<function_traits_u<decltype(&test_function)>::return_type>();
  print_type<function_traits_u<decltype(&test_function)>::arguments_type>();
  CINCH_CAPTURE() << std::endl;

  // &Class::f(...) [const] [volatile]
  print_type<function_traits_u<decltype(&type_t::mem)>::return_type>();
  print_type<function_traits_u<decltype(&type_t::mem)>::arguments_type>();
  print_type<function_traits_u<decltype(&type_t::mem)>::owner_type>();
  print_type<function_traits_u<decltype(&type_t::memc)>::return_type>();
  print_type<function_traits_u<decltype(&type_t::memc)>::arguments_type>();
  print_type<function_traits_u<decltype(&type_t::memc)>::owner_type>();
  print_type<function_traits_u<decltype(&type_t::memv)>::return_type>();
  print_type<function_traits_u<decltype(&type_t::memv)>::arguments_type>();
  print_type<function_traits_u<decltype(&type_t::memv)>::owner_type>();
  print_type<function_traits_u<decltype(&type_t::memcv)>::return_type>();
  print_type<function_traits_u<decltype(&type_t::memcv)>::arguments_type>();
  print_type<function_traits_u<decltype(&type_t::memcv)>::owner_type>();
  CINCH_CAPTURE() << std::endl;

  print_type<function_traits_u<
    std::function<decltype(test_function)>>::return_type>();
  print_type<function_traits_u<
    std::function<decltype(test_function)>>::arguments_type>();
  CINCH_CAPTURE() << std::endl;

  type_t type_instance;

  type_t & flr = type_instance;
  const type_t & flrc = type_instance;
  volatile type_t & flrv = type_instance;
  const volatile type_t & flrcv = type_instance;

  type_t && frr = type_t{};
  const type_t && frrc = type_t{};
  volatile type_t && frrv = type_t{};
  const volatile type_t && frrcv = type_t{};

  print_type<function_traits_u<decltype(flr)>::return_type>();
  print_type<function_traits_u<decltype(flr)>::arguments_type>();
  print_type<function_traits_u<decltype(flrc)>::return_type>();
  print_type<function_traits_u<decltype(flrc)>::arguments_type>();
  print_type<function_traits_u<decltype(flrv)>::return_type>();
  print_type<function_traits_u<decltype(flrv)>::arguments_type>();
  print_type<function_traits_u<decltype(flrcv)>::return_type>();
  print_type<function_traits_u<decltype(flrcv)>::arguments_type>();
  print_type<function_traits_u<decltype(frr)>::return_type>();
  print_type<function_traits_u<decltype(frr)>::arguments_type>();
  print_type<function_traits_u<decltype(frrc)>::return_type>();
  print_type<function_traits_u<decltype(frrc)>::arguments_type>();
  print_type<function_traits_u<decltype(frrv)>::return_type>();
  print_type<function_traits_u<decltype(frrv)>::arguments_type>();
  print_type<function_traits_u<decltype(frrcv)>::return_type>();
  print_type<function_traits_u<decltype(frrcv)>::arguments_type>();
  CINCH_CAPTURE() << std::endl;

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
