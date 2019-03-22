#include <flecsi/utils/ftest.h>
#include <flecsi/utils/tuple_wrapper.h>

int
tuple_wrapper(int argc, char ** argv) {

  FTEST();

  // generic_tuple_t
  flecsi::utils::generic_tuple_t a;
  (void)a;

  // tuple_wrapper_
  flecsi::utils::tuple_wrapper_<> b;
  flecsi::utils::tuple_wrapper_<char> c('c');
  ;
  flecsi::utils::tuple_wrapper_<int, double> d(1, 2.0);
  flecsi::utils::tuple_wrapper_<float, char, int> e(1.0f, 'x', 2);

  // tuple_wrapper_::tuple_t
#ifdef __GNUG__
  EXPECT_EQ(
    (flecsi::utils::type<
      typename flecsi::utils::tuple_wrapper_<float, char, int>::tuple_t>()),
    "std::tuple<float, char, int>");
#endif

  // tuple_wrapper_::get
  EXPECT_EQ(c.get<0>(), 'c');
  EXPECT_EQ(d.get<0>(), 1);
  EXPECT_EQ(d.get<1>(), 2.0);
  EXPECT_EQ(e.get<0>(), 1.0f);
  EXPECT_EQ(e.get<1>(), 'x');
  EXPECT_EQ(e.get<2>(), 2);

  return 0;
}

ftest_register_test(tuple_wrapper);
