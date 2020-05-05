#include "flecsi/util/tuple_walker.hh"

// struct once
struct once : public flecsi::util::tuple_walker<once> {
  void handle(const double d) const {
    CINCH_CAPTURE() << d << std::endl;
  }
};

// struct twice
struct twice : public flecsi::util::tuple_walker<twice> {
  void handle(const double d) {
    CINCH_CAPTURE() << 2 * d << std::endl;
  }
};

// struct thrice
struct thrice : public flecsi::util::tuple_walker<thrice> {
  void handle(double d) const {
    CINCH_CAPTURE() << 3 * d << std::endl;
  }
};

int
tuple_walker() {
  UNIT {
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
  };
} // tuple_walker

flecsi::unit::driver<tuple_walker> driver;
