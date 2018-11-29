/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/


#include <iostream>

#include <cinchtest.h>

#include <flecsi/utils/tuple_walker.h>

// struct once
struct once : public flecsi::utils::tuple_walker_u<once> {
  void handle(const double d) const {
    CINCH_CAPTURE() << d << std::endl;
  }
};

// struct twice
struct twice : public flecsi::utils::tuple_walker_u<twice> {
  void handle(const double d) {
    CINCH_CAPTURE() << 2 * d << std::endl;
  }
};

// struct thrice
struct thrice : public flecsi::utils::tuple_walker_u<thrice> {
  void handle(double d) const {
    CINCH_CAPTURE() << 3 * d << std::endl;
  }
};

TEST(tuple_walker, all) {
  std::tuple<> nothing;
  std::tuple<int, float, double> t(1, float(2), double(3));

  // walk tuples via struct once
  once a;
  a.walk(nothing);
  a.walk(t);
  CINCH_CAPTURE() << std::endl;

  // walk tuples via struct twice
  twice b;
  b.walk(nothing);
  b.walk(t);
  CINCH_CAPTURE() << std::endl;

  // walk tuples via struct thrice
  thrice c;
  c.walk(nothing);
  c.walk(t);

  // compare
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("tuple_walker.blessed"));

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
