/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/common.h>
#include <flecsi/utils/id.h>
#include <flecsi/utils/test/print_type.h>

// includes: C++
#include <vector>

// includes: other
#include <cinchtest.h>

// =============================================================================
// Helper constructs
// =============================================================================

// print
// Print a flecsi::utils::id_.
template<std::size_t P, std::size_t E>
void
print(const flecsi::utils::id_<P, E> & value) {
  CINCH_CAPTURE() << "\ndimension == " << value.dimension();
  CINCH_CAPTURE() << "\ndomain    == " << value.domain();
  CINCH_CAPTURE() << "\npartition == " << value.partition();
  CINCH_CAPTURE() << "\nentity    == " << value.entity();
  CINCH_CAPTURE() << std::endl;
}

// binary
// Write parameter as binary.
// Assumes parameter is of an unsigned integral type.
// Second parameter is number of bits to print.
template<class T>
std::string
binary(const T value, const std::size_t nbit = sizeof(T) * CHAR_BIT) {
  std::string s;
  for(std::size_t i = nbit; i--;)
    s += value >> i & T(1) ? '1' : '0';
  return s;
}

// identical
// Check that the two id_s have identical content.
template<std::size_t PBITS, std::size_t EBITS>
inline bool
identical(const flecsi::utils::id_<PBITS, EBITS> & lhs,
  const flecsi::utils::id_<PBITS, EBITS> & rhs) {
  return lhs.dimension() == rhs.dimension() && lhs.domain() == rhs.domain() &&
         lhs.partition() == rhs.partition() && lhs.entity() == rhs.entity();
}

// =============================================================================
// Exercise all of id.h's constructs
// =============================================================================

// Numbers of bits for our tests.
// Note: we really should test with far more than just one set of these.
#define PBITS 20 /* for partition */
#define EBITS 40 /* for entity    */

// TEST
TEST(id, all) {

  // type: id == id_<PBITS,EBITS>
  using id = flecsi::utils::id_<PBITS, EBITS>;
  using flecsi::utils::local_id_t;

  // ------------------------
  // misc
  // ------------------------

  // local_id_t
  /// print_type<flecsi::utils::local_id_t>();
  CINCH_CAPTURE() << "sizeof(flecsi::utils::local_id_t) == "
                  << sizeof(flecsi::utils::local_id_t) << "\n";
  CINCH_CAPTURE() << std::endl;

  // These here just exercise my binary() function...
  CINCH_CAPTURE() << binary(0u) << std::endl;
  CINCH_CAPTURE() << binary(1u) << std::endl;
  CINCH_CAPTURE() << binary(2u) << std::endl;
  CINCH_CAPTURE() << binary(3u) << std::endl;
  CINCH_CAPTURE() << binary(4u) << std::endl;
  CINCH_CAPTURE() << binary(5u) << std::endl;
  CINCH_CAPTURE() << binary(6u) << std::endl;
  CINCH_CAPTURE() << binary(7u) << std::endl;
  CINCH_CAPTURE() << binary(8u) << std::endl;
  CINCH_CAPTURE() << std::endl;

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
    // Arguments: (local_id,partition_id)
    const id b = id::make<1, 2>(3, 4);
    print(b);
  }

  // ------------------------
  // local_id
  // ------------------------

  {
    const id a = id::make<1, 2>(3, 4);
    print(a);

    CINCH_CAPTURE() << std::endl;
    CINCH_CAPTURE() << "entity    : " << binary(a.entity(), EBITS) << std::endl;
    CINCH_CAPTURE() << "partition : " << binary(a.partition(), PBITS)
                    << std::endl;
    CINCH_CAPTURE() << "domain    : " << binary(a.domain(), 2) << std::endl;
    CINCH_CAPTURE() << "dimension : " << binary(a.dimension(), 2) << std::endl;

    CINCH_CAPTURE() << "\nlocal_id(): "
                    << binary(a.local_id(), EBITS + PBITS + 2 + 2) << std::endl;

    CINCH_CAPTURE() << std::endl;
    CINCH_CAPTURE() << "local_id()  == " << a.local_id() << std::endl;
    CINCH_CAPTURE() << std::endl;
  }

  // ------------------------
  // assignment
  // ------------------------

  {
    id a = id::make<1, 2>(3, 4);
    id b = id{};
    a = b = id{};
    b = a = a;
  }

  // ------------------------
  // getters (accessors)
  // ------------------------

  {
    id a = id::make<1, 2>(3, 4);

    CINCH_CAPTURE() << a.dimension() << std::endl;
    CINCH_CAPTURE() << a.domain() << std::endl;
    CINCH_CAPTURE() << a.partition() << std::endl;
    CINCH_CAPTURE() << a.entity() << std::endl;
    CINCH_CAPTURE() << std::endl;

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
    //    this->local_id() == id.local_id()
    //
    // != returns:
    //    !(==)
    //
    // Where:
    //    local_id() == bits from: [entity partition domain dimension]

    // <dimension,domain>(entity,partition)...
    const id a = id::make<2, 3>(10, 20);
    const id b = id::make<2, 3>(50, 60);
    const id c = id::make<2, 3>(50, 60);

    CINCH_CAPTURE() << a.local_id() << std::endl;
    CINCH_CAPTURE() << b.local_id() << std::endl;
    CINCH_CAPTURE() << std::endl;

    // <
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(!(b < a));

    // ==
    // The present operator== returns true iff the LHS and RHS have the same
    // [entity partition domain dimension] bits.
    CINCH_CAPTURE() << binary(a.local_id()) << std::endl;
    CINCH_CAPTURE() << std::endl;

    CINCH_CAPTURE() << binary(b.local_id()) << std::endl;
    CINCH_CAPTURE() << std::endl;

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(b == c);

    // !=
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(b != c);
  }

  // ------------------------
  // ostream << local_id_t
  // ------------------------

  {
    flecsi::utils::local_id_t a = 1, b = 2;
    CINCH_CAPTURE() << a << std::endl;
    CINCH_CAPTURE() << b << std::endl;
  }

  // ------------------------
  // Compare
  // ------------------------

#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("id.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("id.blessed.msvc"));
#else
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("id.blessed"));
#endif
} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
