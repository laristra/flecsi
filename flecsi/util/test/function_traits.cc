#define __FLECSI_PRIVATE__
#include "flecsi/util/function_traits.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/unit.hh"

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
function_traits() {
  UNIT {
    using flecsi::util::function_traits;

    // general
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<type_t>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<type_t>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << std::endl;

    // f(...)
    // &f(...)
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(test_function)>::return_type)
                   << std::endl;
    UNIT_CAPTURE()
      << UNIT_TTYPE(function_traits<decltype(test_function)>::arguments_type)
      << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&test_function)>::return_type)
                   << std::endl;
    UNIT_CAPTURE()
      << UNIT_TTYPE(function_traits<decltype(&test_function)>::arguments_type)
      << std::endl;
    UNIT_CAPTURE() << std::endl;

    // &Class::f(...) [const] [volatile]
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::mem)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::mem)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::mem)>::owner_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memc)>::return_type)
                   << std::endl;
    UNIT_CAPTURE()
      << UNIT_TTYPE(function_traits<decltype(&type_t::memc)>::arguments_type)
      << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memc)>::owner_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE()
      << UNIT_TTYPE(function_traits<decltype(&type_t::memv)>::arguments_type)
      << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memv)>::owner_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memcv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE()
      << UNIT_TTYPE(function_traits<decltype(&type_t::memcv)>::arguments_type)
      << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(&type_t::memcv)>::owner_type)
                   << std::endl;
    UNIT_CAPTURE() << std::endl;

    UNIT_CAPTURE()
      << UNIT_TTYPE(
           function_traits<std::function<decltype(test_function)>>::return_type)
      << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<
                        std::function<decltype(test_function)>>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << std::endl;

    type_t type_instance;

    type_t & flr = type_instance;
    const type_t & flrc = type_instance;
    volatile type_t & flrv = type_instance;
    const volatile type_t & flrcv = type_instance;

    type_t && frr = type_t{};
    const type_t && frrc = type_t{};
    volatile type_t && frrv = type_t{};
    const volatile type_t && frrcv = type_t{};

    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(flr)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(flr)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(flrc)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(flrc)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(flrv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(flrv)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(flrcv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(flrcv)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(frr)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(frr)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(frrc)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(frrc)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(frrv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(frrv)>::arguments_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(function_traits<decltype(frrcv)>::return_type)
                   << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(
                        function_traits<decltype(frrcv)>::arguments_type)
                   << std::endl;

#ifdef __GNUG__
#ifdef __PPC64__
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("function_traits.blessed.ppc"));
#else
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("function_traits.blessed.gnug"));
#endif
#elif defined(_MSC_VER)
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("function_traits.blessed.msvc"));
#else
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("function_traits.blessed"));
#endif
  };
} // function_traits

flecsi::unit::driver<function_traits> driver;
