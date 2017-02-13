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
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/data/old/old_data.h"
#include "flecsi/utils/bitfield.h"

using data_t = flecsi::data::data_t<
  flecsi::data::default_state_user_meta_data_t,
  flecsi::data::default_data_storage_policy_t>;

using flecsi::data::persistent;

TEST(state, sanity) {
  data_t & state = data_t::instance();

  state.register_state<double>("density", 10, 0, 0, persistent);
  state.register_state<double>("pressure", 10, 0, 1, persistent);
  state.register_state<float>("velocity", 15, 0, 0, persistent);
  state.register_global_state<float>("constant", 0, 0, 0x0);

  auto d = state.dense_accessor<double, 0>("density");

  for(auto i: d) {
    d[i] = i;
  } // for

  for(auto i: d) {
    ASSERT_EQ(i, d[i]);
  } // for

  // define a predicate to test for persistent state
  auto pred = [](const auto & a) -> bool {
    flecsi::utils::bitfield_t bf(a.meta().attributes);
    return a.meta().site_id == 0 && bf.bitsset(persistent);
  };

  // get accessors that match type 'double' and predicate
  for(auto a: state.dense_accessors<double>(pred)) {
    std::cout << a.label() << std::endl;
    ASSERT_TRUE(pred(a));
  } // for

  auto c = state.global_accessor<float,0>("constant");
  c = 1.0;
  ASSERT_EQ( 1.0,  *c );
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
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
