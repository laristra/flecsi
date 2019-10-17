#include <flecsi/utils/common.hh>
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/index_space.hh>

int
index_space(int, char **) {

  FTEST();

  FTEST_CAPTURE() << FTEST_TTYPE(flecsi::utils::index_space_t::iterator_t)
                  << std::endl
                  << std::endl;

  // default constructor
  flecsi::utils::index_space_t a;

  // constructor from size
  flecsi::utils::index_space_t b(10);

  // copy constructor
  flecsi::utils::index_space_t c = b;

  // assignment operator
  a = b;

  // operator[], const
  const flecsi::utils::index_space_t d = b;
  FTEST_CAPTURE() << d[0] << std::endl;
  FTEST_CAPTURE() << d[1] << std::endl;
  FTEST_CAPTURE() << d[2] << std::endl;
  FTEST_CAPTURE() << d[3] << std::endl;
  FTEST_CAPTURE() << d[4] << std::endl;
  FTEST_CAPTURE() << std::endl;

  // operator[], non-const
  b[2]; // changes index_ to 2, and returns & to _index (== 2)
  for(auto iter = b.begin(); iter != b.end(); ++iter)
    FTEST_CAPTURE() << *iter << std::endl;
  FTEST_CAPTURE() << std::endl;

  b[2] = 4; // ??? but the 4 isn't accessible, or used for anything
  for(auto iter = b.begin(); iter != b.end(); ++iter)
    FTEST_CAPTURE() << *iter << std::endl;

    // compare
#ifdef __GNUG__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("index_space.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("index_space.blessed.msvc"));
#else
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("index_space.blessed"));
#endif

  return 0;
}

ftest_register_driver(index_space);
