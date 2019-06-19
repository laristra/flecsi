#include <flecsi/utils/debruijn.hh>
#include <flecsi/utils/ftest.hh>

#include <random>

int
debruijn(int argc, char ** argv) {

  FTEST();

  using flecsi::utils::debruijn32_t;

  // index()
  // For: 0, 1, 2
  EXPECT_EQ(debruijn32_t::index(0), 0);
  EXPECT_EQ(debruijn32_t::index(1), 0);
  EXPECT_EQ(debruijn32_t::index(2), 1);

  // index()
  // For:
  //    00000000000000000000000000000001
  //    00000000000000000000000000000010
  //    00000000000000000000000000000100
  //    ...
  //    10000000000000000000000000000000
  for(uint32_t i = 0; i < 32; ++i) {
    EXPECT_EQ(debruijn32_t::index(1 << i), i);
  } // for

  // index()
  // As above, but with 1s placed pseudo-randomly to
  // the left (never to the right) of the existing 1
  std::mt19937 random;
  random.seed(12345);
  for(int count = 0; count < 10000; ++count) {
    for(uint32_t i = 0; i < 32; ++i) {
      EXPECT_EQ(debruijn32_t::index(uint32_t(random() << i) | (1 << i)), i);
    } // for
  } // for

  return 0;
}

ftest_register_driver(debruijn);
