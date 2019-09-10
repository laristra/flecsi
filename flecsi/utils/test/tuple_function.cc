#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/tuple_function.hh>

// struct foo
struct foo {
  // operator(), 0 arguments
  decltype(auto) operator()(void) const {
    return "abc";
  }

  // operator(), 1 argument
  template<class A>
  decltype(auto) operator()(const A & a) const {
    return a + 10;
  }

  // operator(), 2 arguments
  template<class A, class B>
  decltype(auto) operator()(const A & a, const B & b) const {
    return a + b;
  }

  // operator(), 3 arguments
  template<class A, class B, class C>
  decltype(auto) operator()(const A & a, const B & b, const C & c) const {
    return a + b + c;
  }
};

int
tuple_function(int argc, char ** argv) {

  FTEST();

  std::tuple<> zero;
  std::tuple<int> one(1);
  std::tuple<int, float> two(1, float(2));
  std::tuple<int, float, double> three(1, float(2), double(3));

  foo f;
  EXPECT_EQ(flecsi::utils::tuple_function(f, zero), "abc");
  EXPECT_EQ(flecsi::utils::tuple_function(f, one), 11);
  EXPECT_EQ(flecsi::utils::tuple_function(f, two), 3);
  EXPECT_EQ(flecsi::utils::tuple_function(f, three), 6);

  return 0;
}

ftest_register_driver(tuple_function);
