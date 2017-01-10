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

// declare mesh_t for io.h
class fake_mesh_t {};

#include "flecsi/io/io.h"
#include "flecsi/io/io_exodus.h"

namespace flecsi {
namespace io {

// Register file extensions with factory.
bool exodus_g_registered =
io_factory_t<fake_mesh_t>::instance().registerType("g", create_io_exodus<fake_mesh_t>);

bool exodus_exo_registered =
  io_factory_t<fake_mesh_t>::instance().registerType("exo", create_io_exodus<fake_mesh_t>);

// provide empty implementations of read and write.
template<>
int32_t io_exodus_t<fake_mesh_t>::read(const std::string &name, fake_mesh_t &m) {
  return 0;
}
template<>
int32_t io_exodus_t<fake_mesh_t>::write(const std::string &name, fake_mesh_t &m) {
  return 0;
}

// read/write mesh register the .g and .exo suffixes. Call the io.h
// functions which look for those suffix registrations. Calling read/write
// with any other file suffixes will fail, until more are implemented.
TEST(io, readwrite) {
  fake_mesh_t m;
  ASSERT_FALSE(read_mesh("test.g", m));
  ASSERT_FALSE(read_mesh("test.exo", m));
  ASSERT_FALSE(write_mesh("test.g", m));
  ASSERT_FALSE(write_mesh("test.exo", m));
} // TEST

} // namespace io
} // namespace flecsi

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
