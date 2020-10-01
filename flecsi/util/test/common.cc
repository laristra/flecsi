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

#define __FLECSI_PRIVATE__
#include "flecsi/util/common.hh"
#include "flecsi/util/constant.hh"
#include "flecsi/util/debruijn.hh"
#include "flecsi/util/demangle.hh"
#include "flecsi/util/function_traits.hh"
#include "flecsi/util/static_verify.hh"
#include "flecsi/util/unit.hh"

#include <random>

using namespace flecsi;

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

template<class T>
using ft = util::function_traits<T>;

template<class A, class B>
constexpr const bool & eq = std::is_same_v<A, B>;

template<class T>
using ret = typename ft<T>::return_type;
template<class T>
using args = typename ft<T>::arguments_type;
template<class... TT>
using tup = std::tuple<TT...>;

template<class T, class R, class A>
constexpr bool
test() {
  static_assert(eq<typename ft<T>::return_type, R>);
  static_assert(eq<typename ft<T>::arguments_type, A>);
  return true;
}
template<class T, class... TT>
constexpr bool
same() {
  return (
    test<TT, typename ft<T>::return_type, typename ft<T>::arguments_type>() &&
    ...);
}
template<auto M>
constexpr bool
pmf() {
  using T = decltype(M);
  static_assert(eq<typename ft<T>::owner_type, MyClass>);
  return test<T, void, tup<char, int>>();
}

static_assert(pmf<&MyClass::mem>());
static_assert(pmf<&MyClass::memc>());
static_assert(pmf<&MyClass::memv>());
static_assert(pmf<&MyClass::memcv>());

static_assert(test<MyClass, int, tup<float, double, long double>>());
static_assert(test<decltype(MyFun), float, tup<double, int, long>>());
static_assert(same<decltype(MyFun),
  decltype(&MyFun),
  decltype(*MyFun),
  std::function<decltype(MyFun)>>());
static_assert(same<MyClass,
  MyClass &,
  const MyClass &,
  volatile MyClass &,
  const volatile MyClass &,
  MyClass &&,
  const MyClass &&,
  volatile MyClass &&,
  const volatile MyClass &&>());

// ---------------
using c31 = util::constants<3, 1>;
static_assert(c31::size == 2);
static_assert(c31::index<1> == 1);
static_assert(c31::index<3> == 0);
static_assert(c31::first == 3);
using c4 = util::constants<4>;
static_assert(c4::value == 4);
static_assert(c4::first == 4);
static_assert(!util::constants<>::size);

// ---------------
/*
  Some classes, with or without members foo and bar.
  These will facilitate our check of the FLECSI_MEMBER_CHECKER macro.
 */

struct first {
  int foo;
};

struct second {
  void bar() {}
};

struct both {
  int foo;
  void bar() {}
};

struct neither {};

// make sure two bars aren't counted as a foo and a bar
struct bars {
  void bar() {}
  void bar(int) {}
};

/*
  We'll be interested in checking classes for the presence or absence
  of members foo and bar. The following macro calls produce constructs
  that will facilitate our doing this.
 */
FLECSI_MEMBER_CHECKER(foo); // Makes has_member_foo<T>. ...Does T have foo?
FLECSI_MEMBER_CHECKER(bar); // Makes has_member_bar<T>. ...Does T have bar?

// first{} has foo only
static_assert(has_member_foo<first>::value);
static_assert(!has_member_bar<first>::value);

// second{} has bar only
static_assert(!has_member_foo<second>::value);
static_assert(has_member_bar<second>::value);

// both{} has both
static_assert(has_member_foo<both>::value);
static_assert(has_member_bar<both>::value);

// neither{} has neither
static_assert(!has_member_foo<neither>::value);
static_assert(!has_member_bar<neither>::value);

// bars{} has two bars, but no foo
static_assert(!has_member_foo<bars>::value);
static_assert(has_member_bar<bars>::value);

// ------------------------
// is_tuple
// ------------------------

// with non-tuple
static_assert(!util::is_tuple<int>::value);

// with tuple
static_assert(util::is_tuple<std::tuple<>>::value);
static_assert(util::is_tuple<std::tuple<int>>::value);
static_assert(util::is_tuple<std::tuple<int, char>>::value);

// ---------------
constexpr bool
debruijn(std::uint32_t x) {
  for(std::uint32_t i = 0; i < 32; ++i)
    if(util::debruijn32_t::index(x << i) != i)
      return false;
  return true;
}

static_assert(util::debruijn32_t::index(0) == 0);
static_assert(debruijn(1));

int
common() {
  UNIT {
    // types
    UNIT_CAPTURE() << UNIT_TTYPE(FLECSI_COUNTER_TYPE) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(flecsi::util::counter_t) << std::endl;
    UNIT_CAPTURE() << std::endl;

    // square
    UNIT_CAPTURE() << flecsi::util::square(10) << std::endl;
    UNIT_CAPTURE() << flecsi::util::square(20.0) << std::endl;
    UNIT_CAPTURE() << std::endl;

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

    UNIT_CAPTURE() << a.next() << std::endl;
    UNIT_CAPTURE() << a.next() << std::endl;
    UNIT_CAPTURE() << a.next() << std::endl;
    UNIT_CAPTURE() << std::endl;

    // unique_name
    // Just exercise; return value generally changes between runs
    const int i = 2;
    const float f = float(3.14);
    EXPECT_NE(flecsi::util::unique_name(&i), "");
    EXPECT_NE(flecsi::util::unique_name(&i), "");
    EXPECT_NE(flecsi::util::unique_name(&f), "");
    EXPECT_NE(flecsi::util::unique_name(&f), "");

    {
      constexpr util::key_array<int, c31> m{};
      static_assert(&m.get<3>() == &m[0]);
      static_assert(&m.get<1>() == &m[1]);
    }

    {
      constexpr util::key_tuple<util::key_type<2, short>,
        util::key_type<8, void *>>
        p{1, nullptr};
      static_assert(p.get<2>() == 1);
      static_assert(p.get<8>() == nullptr);
    }

    {
      std::mt19937 random;
      random.seed(12345);
      for(int n = 10000; n--;)
        EXPECT_TRUE(debruijn(random() | 1));
    }

    {
      // demangle, type
      // The results depend on #ifdef __GNUG__, so we'll just exercise
      // these functions, without checking for particular results.
      EXPECT_NE(flecsi::util::demangle("foo"), "");

      auto str_demangle = UNIT_TTYPE(int);
      auto str_type = flecsi::util::type<int>();

      EXPECT_NE(str_demangle, "");
      EXPECT_NE(str_type, "");
      EXPECT_EQ(str_demangle, str_type);

      const auto sym = flecsi::util::symbol<common>();
#ifdef __GNUG__
      EXPECT_EQ(sym, "common()");
#else
      EXPECT_NE(sym, "");
#endif
    }

    // ------------------------
    // Compare
    // ------------------------

#ifdef __GNUG__
#ifdef __PPC64__
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("common.blessed.ppc"));
#else
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("common.blessed.gnug"));
#endif
#elif defined(_MSC_VER)
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("common.blessed.msvc"));
#else
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("common.blessed"));
#endif
  };
} // common

flecsi::unit::driver<common> driver;
