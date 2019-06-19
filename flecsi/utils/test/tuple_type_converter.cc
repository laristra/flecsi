#include <flecsi/utils/common.hh>
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/tuple_type_converter.hh>

class base
{
};
class derived : public base
{
};
class further : public derived
{
};
class thing
{
};

int
tuple_type_converter(int argc, char ** argv) {

  FTEST();

  // convert_tuple_type_
  using char_t = typename flecsi::utils::convert_tuple_type_<char>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_t) << std::endl;
  using const_char_t =
    typename flecsi::utils::convert_tuple_type_<const char>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(const_char_t) << std::endl;

  FTEST_CAPTURE() << std::endl;

  // convert_tuple_type
  using tuple_t =
    typename flecsi::utils::convert_tuple_type<std::tuple<>>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(tuple_t) << std::endl;
  using tuple_char_t =
    typename flecsi::utils::convert_tuple_type<std::tuple<char>>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(tuple_char_t) << std::endl;
  using tuple_int_double_t =
    typename flecsi::utils::convert_tuple_type<std::tuple<int, double>>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(tuple_int_double_t) << std::endl;

  FTEST_CAPTURE() << std::endl;

  // base_convert_tuple_type_
  using char_int_true_t =
    typename flecsi::utils::base_convert_tuple_type_<char, int, true>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_int_true_t) << std::endl;
  using char_int_false_t =
    typename flecsi::utils::base_convert_tuple_type_<char, int, false>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(char_int_false_t) << std::endl;

  FTEST_CAPTURE() << std::endl;

  // base_convert_tuple_type
  using double_tuple_t = typename flecsi::utils::
    base_convert_tuple_type<base, double, std::tuple<>>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(double_tuple_t) << std::endl;

  using catchall_t =
    typename flecsi::utils::base_convert_tuple_type<base, // use this by
                                                          // default...
      double, // but use this where base is base of tuple element...
      std::tuple<int,
        base, // base is considered to be base
        derived, // base is base
        char,
        thing,
        further // base is base
        >>::type;
  FTEST_CAPTURE() << FTEST_TTYPE(catchall_t) << std::endl;

  // compare
#ifdef __GNUG__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("tuple_type_converter.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("tuple_type_converter.blessed.msvc"));
#endif

  return 0;
}

ftest_register_driver(tuple_type_converter);
