#include <flecsi/utils/common.h>
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/function_traits.h>

struct type_t {
  int operator()(float, double, long double) const {
    return 0;
  }

  void mem(char, int) {}
  void memc(char, int) const {}
  void memv(char, int) volatile {}
  void memcv(char, int) const volatile {}
}; // struct type_t

inline float
test_function(double, int, long) {
  return float(0);
}

int
function_traits(int argc, char ** argv) {

  FTEST();

  using flecsi::utils::function_traits_u;

  // general
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<type_t>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<type_t>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << std::endl;

  // f(...)
  // &f(...)
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(test_function)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(test_function)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&test_function)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(&test_function)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << std::endl;

  // &Class::f(...) [const] [volatile]
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::mem)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(&type_t::mem)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::mem)>::owner_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memc)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(&type_t::memc)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memc)>::owner_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(&type_t::memv)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memv)>::owner_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memcv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE()
    << FTEST_TTYPE(function_traits_u<decltype(&type_t::memcv)>::arguments_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(&type_t::memcv)>::owner_type)
                  << std::endl;
  FTEST_CAPTURE() << std::endl;

  FTEST_CAPTURE()
    << FTEST_TTYPE(
         function_traits_u<std::function<decltype(test_function)>>::return_type)
    << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<
                       std::function<decltype(test_function)>>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << std::endl;

  type_t type_instance;

  type_t & flr = type_instance;
  const type_t & flrc = type_instance;
  volatile type_t & flrv = type_instance;
  const volatile type_t & flrcv = type_instance;

  type_t && frr = type_t{};
  const type_t && frrc = type_t{};
  volatile type_t && frrv = type_t{};
  const volatile type_t && frrcv = type_t{};

  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(flr)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(flr)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(flrc)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(flrc)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(flrv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(flrv)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(flrcv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(flrcv)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(frr)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(frr)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(frrc)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(frrc)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(function_traits_u<decltype(frrv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(frrv)>::arguments_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(frrcv)>::return_type)
                  << std::endl;
  FTEST_CAPTURE() << FTEST_TTYPE(
                       function_traits_u<decltype(frrcv)>::arguments_type)
                  << std::endl;

#ifdef __GNUG__
#ifdef __PPC64__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("function_traits.blessed.ppc"));
#else
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("function_traits.blessed.gnug"));
#endif
#elif defined(_MSC_VER)
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("function_traits.blessed.msvc"));
#else
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("function_traits.blessed"));
#endif

  return 0;
}

ftest_register_driver(function_traits);
