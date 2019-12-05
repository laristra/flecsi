/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// includes: flecsi
#include <flecsi/utils/array_ref.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/test/print_type.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// print_refc
// Prints a span<const char>, as a character string
inline void
print_refc(const flecsi::utils::span<const char> & arr) {
  for(auto c = arr.begin(); c != arr.end(); ++c)
    CINCH_CAPTURE() << *c;
  CINCH_CAPTURE() << std::endl;
}

using flecsi::utils::span;

//=============================================================================
//! \brief Test various constructs in array_ref.h
//=============================================================================

TEST(array_ref, all) {

  using refd = span<const double>;
  using refc = span<const char>;
  using reff = span<const float>;
  using refi = span<const int>;

  CINCH_CAPTURE() << std::endl;

  // ------------------------
  // types
  // ------------------------

  print_type<refd::value_type>();
  print_type<refd::pointer>();
  print_type<refd::reference>();
  print_type<refd::const_reference>();
  print_type<refd::const_iterator>();
  print_type<refd::iterator>();
  print_type<refd::const_reverse_iterator>();
  print_type<refd::reverse_iterator>();
  print_type<refd::size_type>();
  print_type<refd::difference_type>();
  CINCH_CAPTURE() << std::endl;

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
  c = c = b;
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

  // ------------------------
  // substr
  // ------------------------

  // ......................012345678
  const std::string abc = "abcdefghi";

  print_refc(refc(abc));
  CINCH_CAPTURE() << std::endl;

  print_refc(refc(abc).subspan(0));
  print_refc(refc(abc).subspan(1));
  print_refc(refc(abc).subspan(2));
  print_refc(refc(abc).subspan(7));
  print_refc(refc(abc).subspan(8));
  print_refc(refc(abc).subspan(9)); // blank
  CINCH_CAPTURE() << std::endl;

  print_refc(refc(abc).subspan(0, 2));
  print_refc(refc(abc).subspan(1, 4));
  print_refc(refc(abc).subspan(2, 3));
  print_refc(refc(abc).subspan(7, 2));
  print_refc(refc(abc).subspan(8, 1));
  print_refc(refc(abc).subspan(9, 0)); // blank
  CINCH_CAPTURE() << std::endl;

  // ------------------------
  // iterators
  // ------------------------

  for(auto it = abc.begin(); it != abc.end(); ++it)
    CINCH_CAPTURE() << *it;
  CINCH_CAPTURE() << std::endl;
  for(auto it = abc.cbegin(); it != abc.cend(); ++it)
    CINCH_CAPTURE() << *it;
  CINCH_CAPTURE() << std::endl;
  for(auto it = abc.rbegin(); it != abc.rend(); ++it)
    CINCH_CAPTURE() << *it;
  CINCH_CAPTURE() << std::endl;
  for(auto it = abc.crbegin(); it != abc.crend(); ++it)
    CINCH_CAPTURE() << *it;
  CINCH_CAPTURE() << std::endl;

  // ------------------------
  // capacity
  // ------------------------

  const refd cap0;
  const std::vector<double> vec3 = {3.14, 2.18, 2.72};
  const refd cap3 = vec3;

  CINCH_CAPTURE() << cap0.size() << std::endl;
  CINCH_CAPTURE() << (cap0.empty() ? "true" : "false") << std::endl; // empty
  CINCH_CAPTURE() << cap3.size() << std::endl;
  CINCH_CAPTURE() << (cap3.empty() ? "true" : "false") << std::endl; // not
  CINCH_CAPTURE() << std::endl;

  // ------------------------
  // element access
  // ------------------------

  // []
  CINCH_CAPTURE() << cap3[0] << std::endl;
  CINCH_CAPTURE() << cap3[1] << std::endl;
  CINCH_CAPTURE() << cap3[2] << std::endl;
  CINCH_CAPTURE() << std::endl;

  // front(), back()
  CINCH_CAPTURE() << cap3.front() << std::endl;
  CINCH_CAPTURE() << cap3.back() << std::endl;
  CINCH_CAPTURE() << std::endl;

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
    CINCH_CAPTURE() << f.front() << std::endl;
    CINCH_CAPTURE() << f.back() << std::endl;
    CINCH_CAPTURE() << std::endl;

    f = f.last(f.size() - 3);
    f = f.first(f.size() - 2);
    CINCH_CAPTURE() << f.front() << std::endl;
    CINCH_CAPTURE() << f.back() << std::endl;
    CINCH_CAPTURE() << std::endl;

    f = f.last(f.size() - 1); // ==> nothing left!
    EXPECT_EQ(f.size(), 0);
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
    CINCH_CAPTURE() << a.front() << '\n';
    CINCH_CAPTURE() << a.back() << '\n' << std::endl;

    // from T [n]
    const span b(ints);
    CINCH_CAPTURE() << b.front() << '\n';
    CINCH_CAPTURE() << b.back() << '\n' << std::endl;

    // from std::vector
    std::vector<int> ivec(10, 1); // 10 1s
    const span c(ivec);
    CINCH_CAPTURE() << c.front() << '\n';
    CINCH_CAPTURE() << c.back() << '\n' << std::endl;

    // from std::array
    const std::array<int, 2> iarr = {{10, 20}};
    const span d(iarr);
    CINCH_CAPTURE() << d.front() << '\n';
    CINCH_CAPTURE() << d.back() << '\n';
  }

  // ------------------------
  // compare
  // ------------------------
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("array_ref.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("array_ref.blessed.msvc"));
#else
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("array_ref.blessed"));
#endif

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
