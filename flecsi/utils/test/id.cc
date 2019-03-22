/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <flecsi/utils/common.h>
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/id.h>

#include <vector>

template<std::size_t P, std::size_t E, std::size_t F, std::size_t G>
void
print(const flecsi::utils::id_<P, E, F, G> & value) {
  FTEST_CAPTURE() << "\ndimension == " << value.dimension();
  FTEST_CAPTURE() << "\ndomain    == " << value.domain();
  FTEST_CAPTURE() << "\npartition == " << value.partition();
  FTEST_CAPTURE() << "\nentity    == " << value.entity();
  FTEST_CAPTURE() << "\nflags     == " << value.flags();
  FTEST_CAPTURE() << "\nglobal    == " << value.global();
  FTEST_CAPTURE() << std::endl;
}

/*
  Write parameter as binary.
  Assumes parameter is of an unsigned integral type.
  Second parameter is number of bits to print.
 */

template<class T>
std::string
binary(const T value, const std::size_t nbit = sizeof(T) * CHAR_BIT) {
  std::string s;
  for(std::size_t i = nbit; i--;)
    s += value >> i & T(1) ? '1' : '0';
  return s;
}

/*
  Check that the two id_s have identical content.
  This isn't equivalent to id_'s operator==, which at the time of this
  writing does the comparison by using local_id() and FLAGS_UNMASK.
 */
template<std::size_t PBITS,
  std::size_t EBITS,
  std::size_t FBITS,
  std::size_t GBITS>
inline bool
identical(const flecsi::utils::id_<PBITS, EBITS, FBITS, GBITS> & lhs,
  const flecsi::utils::id_<PBITS, EBITS, FBITS, GBITS> & rhs) {
  return lhs.dimension() == rhs.dimension() && lhs.domain() == rhs.domain() &&
         lhs.partition() == rhs.partition() && lhs.entity() == rhs.entity() &&
         lhs.flags() == rhs.flags() && lhs.global() == rhs.global();
}

// Numbers of bits for our tests.
// Note: we really should test with far more than just one set of these.
#define PBITS 20 /* for partition */
#define EBITS 40 /* for entity    */
#define FBITS 4 /* for flags     */
#define GBITS 60 /* for global    */

int
id(int argc, char ** argv) {

  FTEST();

  // type: id == id_<PBITS,EBITS,FBITS,GBITS>
  using id = flecsi::utils::id_<PBITS, EBITS, FBITS, GBITS>;
  using flecsi::utils::local_id_t;

  // ------------------------
  // misc
  // ------------------------

  // local_id_t
  FTEST_CAPTURE() << FTEST_TTYPE(flecsi::utils::local_id_t) << std::endl;
  FTEST_CAPTURE() << "sizeof(flecsi::utils::local_id_t) == "
                  << sizeof(flecsi::utils::local_id_t) << std::endl;

  // id::FLAGS_UNMASK
  FTEST_CAPTURE() << "FLAGS_UNMASK == " << id::FLAGS_UNMASK << '\n';
  FTEST_CAPTURE() << "FLAGS_UNMASK == " << binary(id::FLAGS_UNMASK) << '\n';
  FTEST_CAPTURE() << std::endl;

  // These here just exercise my binary() function...
  FTEST_CAPTURE() << binary(0u) << std::endl;
  FTEST_CAPTURE() << binary(1u) << std::endl;
  FTEST_CAPTURE() << binary(2u) << std::endl;
  FTEST_CAPTURE() << binary(3u) << std::endl;
  FTEST_CAPTURE() << binary(4u) << std::endl;
  FTEST_CAPTURE() << binary(5u) << std::endl;
  FTEST_CAPTURE() << binary(6u) << std::endl;
  FTEST_CAPTURE() << binary(7u) << std::endl;
  FTEST_CAPTURE() << binary(8u) << std::endl;
  FTEST_CAPTURE() << std::endl;

  // ------------------------
  // constructors
  // ------------------------

  {
    id a;
    const id b(id{});
    (void)b;
    const id c = a;
    const id d(12345);
    print(d);
  }

  // ------------------------
  // make
  // ------------------------

  {
    // make<DIMENSION,DOMAIN>
    // Arguments: (local_id [,partition_id [,flags [,global]]])
    const id a = id::make<1, 2>(3);
    const id b = id::make<1, 2>(3, 4);
    const id c = id::make<1, 2>(3, 4, 5);
    const id d = id::make<1, 2>(3, 4, 5, 6);
    print(a);
    print(b);
    print(c);
    print(d);

    // make<DOMAIN>
    // Arguments: (dimension, local_id [,partition_id [,flags [,global]]])
    const id e = id::make<2>(1, 3);
    const id f = id::make<2>(1, 3, 4);
    const id g = id::make<2>(1, 3, 4, 5);
    const id h = id::make<2>(1, 3, 4, 5, 6);
    print(e);
    print(f);
    print(g);
    print(h);

    EXPECT_TRUE(identical(a, e));
    EXPECT_TRUE(identical(b, f));
    EXPECT_TRUE(identical(c, g));
    EXPECT_TRUE(identical(d, h));
  }

  // ------------------------
  // local_id
  // global_id
  // ------------------------

  {
    const id a = id::make<1, 2>(3, 4, 5, 6);
    print(a);

    FTEST_CAPTURE() << std::endl;
    FTEST_CAPTURE() << "entity    : " << binary(a.entity(), EBITS) << std::endl;
    FTEST_CAPTURE() << "partition : " << binary(a.partition(), PBITS)
                    << std::endl;
    FTEST_CAPTURE() << "domain    : " << binary(a.domain(), 2) << std::endl;
    FTEST_CAPTURE() << "dimension : " << binary(a.dimension(), 2) << std::endl;

    FTEST_CAPTURE() << "\nlocal_id(): "
                    << binary(a.local_id(), EBITS + PBITS + 2 + 2) << std::endl;

    FTEST_CAPTURE() << std::endl;
    FTEST_CAPTURE() << "local_id()  == " << a.local_id() << std::endl;
    FTEST_CAPTURE() << "global_id() == " << a.global_id() << std::endl;
    FTEST_CAPTURE() << std::endl;
  }

  // ------------------------
  // assignment
  // ------------------------

  {
    id a = id::make<1, 2>(3, 4, 5, 6);
    id b = id{};
    a = b = id{};
    b = a = a;
  }

  // ------------------------
  // setters
  // ------------------------

  {
    id a = id::make<1, 2>(3, 4, 5, 6);
    print(a);
    a.set_global(100);
    a.set_partition(200);
    a.set_flags(15);
    print(a);
  }

  // ------------------------
  // getters (accessors)
  // ------------------------

  {
    id a = id::make<1, 2>(3, 4, 5, 6);

    FTEST_CAPTURE() << a.dimension() << std::endl;
    FTEST_CAPTURE() << a.domain() << std::endl;
    FTEST_CAPTURE() << a.partition() << std::endl;
    FTEST_CAPTURE() << a.entity() << std::endl;
    FTEST_CAPTURE() << a.flags() << std::endl;
    FTEST_CAPTURE() << a.global() << std::endl;
    FTEST_CAPTURE() << std::endl;

    EXPECT_EQ(a.entity(), a.index_space_index());
  }

  // ------------------------
  // <, ==, !=
  // ------------------------

  {
    // <  returns:
    //    this->local_id() < id.local_id()
    //
    // == returns:
    //    (this->local_id() & FLAGS_UNMASK) ==
    //    (   id.local_id() & FLAGS_UNMASK)
    //
    // != returns:
    //    !(this->local_id() == id.local_id())
    //
    // Where:
    //    local_id() == bits from: [entity partition domain dimension]

    // <dimension,domain>(entity,partition,flags,global)...
    const id a = id::make<2, 3>(10, 20, 30, 40);
    const id b = id::make<2, 3>(50, 60, 70, 80);
    const id c = id::make<2, 3>(50, 60, 7000, 8000);

    FTEST_CAPTURE() << a.local_id() << std::endl;
    FTEST_CAPTURE() << b.local_id() << std::endl;
    FTEST_CAPTURE() << std::endl;

    // <
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(!(b < a));

    // ==
    // The present operator== returns true iff the LHS and RHS have the same
    // [entity partition domain dimension] bits when &ed with FLAGS_UNMASK.
    // Note: my (Martin's) analysis suggests that large values of entity have
    // high-order bits that would run into the FBITS 0s in FLAGS_UNMASK. So,
    // I should clarify if the definition is really as it was intended.
    FTEST_CAPTURE() << binary(a.local_id()) << std::endl;
    FTEST_CAPTURE() << binary(local_id_t(id::FLAGS_UNMASK)) << std::endl;
    FTEST_CAPTURE() << binary(a.local_id() & id::FLAGS_UNMASK) << std::endl;
    FTEST_CAPTURE() << std::endl;

    FTEST_CAPTURE() << binary(b.local_id()) << std::endl;
    FTEST_CAPTURE() << binary(local_id_t(id::FLAGS_UNMASK)) << std::endl;
    FTEST_CAPTURE() << binary(b.local_id() & id::FLAGS_UNMASK) << std::endl;
    FTEST_CAPTURE() << std::endl;

    // Not really fully testing the operator here...
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(b == c);

    // !=
    // At the moment, this isn't quite the same as !(==). I'm unsure
    // as to why it's defined the way it is. -Martin

    // Not really fully testing the operator here...
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(b != c);
  }

  // ------------------------
  // ostream << local_id_t
  // ------------------------

  {
    flecsi::utils::local_id_t a = 1, b = 2;
    FTEST_CAPTURE() << a << std::endl;
    FTEST_CAPTURE() << b << std::endl;
  }

  // ------------------------
  // Compare
  // ------------------------

#ifdef __GNUG__
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("id.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("id.blessed.msvc"));
#else
  EXPECT_TRUE(FTEST_EQUAL_BLESSED("id.blessed"));
#endif

  return 0;
}

ftest_register_test(id);
