/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include <flecsi/utils/common.h>
#include <flecsi/utils/index_space.h>
#include <flecsi/utils/test/print_type.h>

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>

// =============================================================================
// Test various aspects of flecsi::utils::index_space
// =============================================================================

TEST(index_space, all) {
  // iterator_t
  print_type<flecsi::utils::index_space_t::iterator_t>();
  CINCH_CAPTURE() << std::endl;

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
  CINCH_CAPTURE() << d[0] << std::endl;
  CINCH_CAPTURE() << d[1] << std::endl;
  CINCH_CAPTURE() << d[2] << std::endl;
  CINCH_CAPTURE() << d[3] << std::endl;
  CINCH_CAPTURE() << d[4] << std::endl;
  CINCH_CAPTURE() << std::endl;

  // operator[], non-const
  b[2]; // changes index_ to 2, and returns & to _index (== 2)
  for(auto iter = b.begin(); iter != b.end(); ++iter)
    CINCH_CAPTURE() << *iter << std::endl;
  CINCH_CAPTURE() << std::endl;

  b[2] = 4; // ??? but the 4 isn't accessible, or used for anything
  for(auto iter = b.begin(); iter != b.end(); ++iter)
    CINCH_CAPTURE() << *iter << std::endl;

    // compare
#ifdef __GNUG__
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("index_space.blessed.gnug"));
#elif defined(_MSC_VER)
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("index_space.blessed.msvc"));
#else
  EXPECT_TRUE(CINCH_EQUAL_BLESSED("index_space.blessed"));
#endif

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
