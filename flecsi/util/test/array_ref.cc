#include "flecsi/util/array_ref.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/unit.hh"

#include <iostream>
#include <numeric>

// Prints a span<const char>, as a character string
inline void
print_refc(const flecsi::util::span<const char> & arr) {
  for(auto c = arr.begin(); c != arr.end(); ++c)
    UNIT_CAPTURE() << *c;
  UNIT_CAPTURE() << std::endl;
}

using flecsi::util::span;

int
array_ref() {
  UNIT {
    using refd = span<const double>;
    using refc = span<const char>;
    using reff = span<const float>;
    using refi = span<const int>;

    UNIT_CAPTURE() << std::endl;

    // ------------------------
    // types
    // ------------------------

    UNIT_CAPTURE() << UNIT_TTYPE(refd::value_type) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::pointer) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::reference) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::const_reference) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::const_iterator) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::iterator) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::const_reverse_iterator) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::reverse_iterator) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::size_type) << std::endl;
    UNIT_CAPTURE() << UNIT_TTYPE(refd::difference_type) << std::endl;
    UNIT_CAPTURE() << std::endl;

    // ------------------------
    // constructor/assignment
    // ------------------------

    // default constructor
    refd a;
    EXPECT_EQ(a.begin(), nullptr);
    EXPECT_EQ(a.end(), nullptr);

    // constructor from pointer and length
    const std::size_t length = 5;
    double plain_array[length];
    refd b(plain_array, length);
    EXPECT_EQ(b.begin(), &plain_array[0]);
    EXPECT_EQ(b.end(), &plain_array[0] + length);

    // copy constructor
    refd c = b;
    EXPECT_EQ(c.begin(), &plain_array[0]);
    EXPECT_EQ(c.end(), &plain_array[0] + length);

    // copy assignment
    c = b;
    EXPECT_EQ(c.begin(), &plain_array[0]);
    EXPECT_EQ(c.end(), &plain_array[0] + length);

    // ------------------------
    // conversion
    // ------------------------

    // conversion from std::vector
    const std::vector<double> vec = {3.14, 2.18, 2.72};
    refd d = vec;
    EXPECT_EQ(d.begin(), &vec[0]);
    EXPECT_EQ(d.end(), &vec[0] + vec.size());

    // conversion from std::string
    const std::string str = "hello";
    const refc e = str;
    EXPECT_EQ(e.begin(), &str[0]);
    EXPECT_EQ(e.end(), &str[0] + str.size());

    // conversion from C-style array
    const std::size_t arrlen = 9;
    const float arr[arrlen] = {1, 9, 2, 8, 3, 7, 4, 6, 5};
    const reff f = arr;
    EXPECT_EQ(f.begin(), &arr[0]);
    EXPECT_EQ(f.end(), &arr[0] + arrlen);

    // conversion from std::array
    const std::array<int, 2> ints = {{10, 20}};
    const refi g = ints;
    EXPECT_EQ(g.begin(), &ints[0]);
    EXPECT_EQ(g.end(), &ints[0] + ints.size());

    span<double> sd(plain_array);
    EXPECT_EQ(refd(sd).data(), sd.data()); // conversion from non-const span
    EXPECT_EQ(refd(span<double>()).size(), 0u); // ...from rvalue span

    // ------------------------
    // substr
    // ------------------------

    // ......................012345678
    const std::string abc = "abcdefghi";

    print_refc(refc(abc));
    UNIT_CAPTURE() << std::endl;

    print_refc(refc(abc).subspan(0));
    print_refc(refc(abc).subspan(1));
    print_refc(refc(abc).subspan(2));
    print_refc(refc(abc).subspan(7));
    print_refc(refc(abc).subspan(8));
    print_refc(refc(abc).subspan(9)); // blank
    UNIT_CAPTURE() << std::endl;

    print_refc(refc(abc).subspan(0, 2));
    print_refc(refc(abc).subspan(1, 4));
    print_refc(refc(abc).subspan(2, 3));
    print_refc(refc(abc).subspan(7, 2));
    print_refc(refc(abc).subspan(8, 1));
    print_refc(refc(abc).subspan(9, 0)); // blank
    UNIT_CAPTURE() << std::endl;

    // ------------------------
    // iterators
    // ------------------------

    for(auto it = abc.begin(); it != abc.end(); ++it)
      UNIT_CAPTURE() << *it;
    UNIT_CAPTURE() << std::endl;
    for(auto it = abc.cbegin(); it != abc.cend(); ++it)
      UNIT_CAPTURE() << *it;
    UNIT_CAPTURE() << std::endl;
    for(auto it = abc.rbegin(); it != abc.rend(); ++it)
      UNIT_CAPTURE() << *it;
    UNIT_CAPTURE() << std::endl;
    for(auto it = abc.crbegin(); it != abc.crend(); ++it)
      UNIT_CAPTURE() << *it;
    UNIT_CAPTURE() << std::endl;

    // ------------------------
    // capacity
    // ------------------------

    const refd cap0;
    const std::vector<double> vec3 = {3.14, 2.18, 2.72};
    const refd cap3 = vec3;

    UNIT_CAPTURE() << cap0.size() << std::endl;
    UNIT_CAPTURE() << (cap0.empty() ? "true" : "false") << std::endl; // empty
    UNIT_CAPTURE() << cap3.size() << std::endl;
    UNIT_CAPTURE() << (cap3.empty() ? "true" : "false") << std::endl; // not
    UNIT_CAPTURE() << std::endl;

    // ------------------------
    // element access
    // ------------------------
    // []
    UNIT_CAPTURE() << cap3[0] << std::endl;
    UNIT_CAPTURE() << cap3[1] << std::endl;
    UNIT_CAPTURE() << cap3[2] << std::endl;
    UNIT_CAPTURE() << std::endl;

    // front(), back()
    UNIT_CAPTURE() << cap3.front() << std::endl;
    UNIT_CAPTURE() << cap3.back() << std::endl;
    UNIT_CAPTURE() << std::endl;

    // data()
    EXPECT_NE(cap3.data(), nullptr);

    // ------------------------
    // outgoing conversion
    // ------------------------

    // to std::vector
    EXPECT_EQ(vec3, to_vector(cap3));

    // to std::string
    EXPECT_EQ(abc, std::string(refc(abc).begin(), refc(abc).end()));

    // ------------------------
    // mutators
    // ------------------------

    {
      // remove_*()
      int iarray[9] = {1, 9, 2, 8, 3, 7, 4, 6, 5};
      refi f = iarray;
      f = f.last(f.size() - 2);
      f = f.first(f.size() - 1);
      UNIT_CAPTURE() << f.front() << std::endl;
      UNIT_CAPTURE() << f.back() << std::endl;
      UNIT_CAPTURE() << std::endl;

      f = f.last(f.size() - 3);
      f = f.first(f.size() - 2);
      UNIT_CAPTURE() << f.front() << std::endl;
      UNIT_CAPTURE() << f.back() << std::endl;
      UNIT_CAPTURE() << std::endl;

      f = f.last(f.size() - 1); // ==> nothing left!
      EXPECT_EQ(f.size(), 0u);
      for(auto it = f.begin(); it != f.end(); ++it)
        assert(false);
    }

    // ------------------------
    // CTAD
    // ------------------------

    {
      // from * and length
      const std::size_t length = 9;
      int ints[length] = {1, 9, 2, 8, 3, 7, 4, 6, 5};
      const span a(&ints[0], length);
      UNIT_CAPTURE() << a.front() << '\n';
      UNIT_CAPTURE() << a.back() << '\n' << std::endl;

      // from T [n]
      const span b(ints);
      UNIT_CAPTURE() << b.front() << '\n';
      UNIT_CAPTURE() << b.back() << '\n' << std::endl;

      // from std::vector
      std::vector<int> ivec(10, 1); // 10 1s
      const span c(ivec);
      UNIT_CAPTURE() << c.front() << '\n';
      UNIT_CAPTURE() << c.back() << '\n' << std::endl;

      // from std::array
      const std::array<int, 2> iarr = {{10, 20}};
      const span d(iarr);
      UNIT_CAPTURE() << d.front() << '\n';
      UNIT_CAPTURE() << d.back() << '\n';
    }
    // ------------------------
    // compare
    // ------------------------
#ifdef __GNUG__
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("array_ref.blessed.gnug"));
#elif defined(_MSC_VER)
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("array_ref.blessed.msvc"));
#else
    EXPECT_TRUE(UNIT_EQUAL_BLESSED("array_ref.blessed"));
#endif

    const flecsi::util::iota_view gap(24, 29); // between primes
    EXPECT_EQ(std::accumulate(gap.begin(), gap.end(), 0),
      (gap.front() + gap.back()) * gap.size() / 2);

    flecsi::util::transform_view tv(b, [](auto & x) { return &x; });
    EXPECT_EQ(*tv.begin(), &b.front());
    EXPECT_EQ(tv.end()[-1], &b.back());
  };
} // array_ref

flecsi::unit::driver<array_ref> driver;
