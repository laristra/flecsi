/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <unordered_map>

#include "flecsi/execution/default_driver.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data.h"


using namespace flecsi::data;

static const size_t N = 100000;

// Create a derived type so that we can instantiate a data_client_t.
struct derived_t : public data_client_t
{
  derived_t() : data_client_t() {}
}; // derived_t

TEST(data_client, sanity) {

  // Map to store runtime ids.
  std::unordered_map<uintptr_t, int> map;

  // Array of data client
  derived_t dc[N];

  for(size_t i(0); i<N; ++i) {

    // This tests that the runtime id is unique over a range
    // of addresses. The emplace function of the map returns
    // a std::pair<iterator, bool>. If the boolean value is
    // true, the emplace resulted in a new element in the map,
    // i.e., the runtime_id was unique. Otherwise, it returns
    // false, meaning that the element already exists and the
    // id that was created is not unique.
    auto itr = map.emplace(std::move(dc[i].runtime_id()), i);

    CINCH_ASSERT(TRUE, itr.second);
  } // for

} // TEST


//! \brief Tests the data_client's destructor
TEST(data_client, destructor) {

  // create a new data_client
  auto dc = new derived_t;

  // Register data
  flecsi_register_data(*dc, hydro, pressure, double, global, 1);
  flecsi_register_data(*dc, hydro, density, double, global, 1);

  // get all accessors to the data
  auto accs = flecsi_get_handles(*dc, hydro, double, global, 0);
  
  ASSERT_EQ( accs.size(), 2 );
  ASSERT_EQ( accs[0].label(), "density" );
  ASSERT_EQ( accs[1].label(), "pressure" );

  // store the runtime id
  auto rid = dc->runtime_id();

  ASSERT_EQ( flecsi::data::storage_t::instance().count(rid), 2);

  // delete the data_client
  delete dc;

  // make sure the data is gone
  ASSERT_EQ( flecsi::data::storage_t::instance().count(rid), 0 );

} // TEST

//! \brief Tests the data_client's move operator
TEST(data_client, move) {

  // create new data_clients
  derived_t dc1, dc2;
  auto rid1 = dc1.runtime_id();

  // Register data
  flecsi_register_data(dc1, hydro, pressure, double, global, 1);
  flecsi_register_data(dc1, hydro, density, double, global, 1);

  // get all accessors to the data
  {
    auto accs = flecsi_get_handles(dc1, hydro, double, global, 0);
  
    ASSERT_EQ( accs.size(), 2 );
    ASSERT_EQ( accs[0].label(), "density" );
    ASSERT_EQ( accs[1].label(), "pressure" );
  } // scope

  // create a new data client
  dc2 = std::move(dc1);
  auto rid2 = dc1.runtime_id();

  // make sure the data is gone from data client 1
  ASSERT_EQ( flecsi::data::storage_t::instance().count(rid1), 0 );

  // it should show up in the new data client though
  {
    auto accs = flecsi_get_handles(dc2, hydro, double, global, 0);
  
    ASSERT_EQ( accs.size(), 2 );
    ASSERT_EQ( accs[0].label(), "density" );
    ASSERT_EQ( accs[1].label(), "pressure" );
  } // scope

} // TEST


/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 *  CINCH_ASSERT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 *  CINCH_EXPECT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
