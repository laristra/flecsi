#include "flecsi/util/tuple_type_converter.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/unit.hh"

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
tuple_type_converter() {
  UNIT {
    using namespace flecsi::util;

    // convert_tuple_type_
    using char_t = typename convert_tuple_type_<char>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(char_t) << std::endl;
    using const_char_t = typename convert_tuple_type_<const char>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(const_char_t) << std::endl;

    UNIT_CAPTURE() << std::endl;

    // convert_tuple_type
    using tuple_t = typename convert_tuple_type<std::tuple<>>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(tuple_t) << std::endl;
    using tuple_char_t = typename convert_tuple_type<std::tuple<char>>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(tuple_char_t) << std::endl;
    using tuple_int_double_t =
      typename convert_tuple_type<std::tuple<int, double>>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(tuple_int_double_t) << std::endl;

    UNIT_CAPTURE() << std::endl;

    // base_convert_tuple_type_
    using char_int_true_t =
      typename base_convert_tuple_type_<char, int, true>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(char_int_true_t) << std::endl;
    using char_int_false_t =
      typename base_convert_tuple_type_<char, int, false>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(char_int_false_t) << std::endl;

    UNIT_CAPTURE() << std::endl;

    // base_convert_tuple_type
    using double_tuple_t =
      typename base_convert_tuple_type<base, double, std::tuple<>>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(double_tuple_t) << std::endl;

    using catchall_t = typename base_convert_tuple_type<base, // use this by
                                                              // default...
      double, // but use this where base is base of tuple element...
      std::tuple<int,
        base, // base is considered to be base
        derived, // base is base
        char,
        thing,
        further // base is base
        >>::type;
    UNIT_CAPTURE() << UNIT_TTYPE(catchall_t) << std::endl;

    // compare
#ifdef __GNUG__
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("tuple_type_converter.blessed.gnug"));
#elif defined(_MSC_VER)
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("tuple_type_converter.blessed.msvc"));
#endif
  };
} // tuple_type_converter

flecsi::unit::driver<tuple_type_converter> driver;
