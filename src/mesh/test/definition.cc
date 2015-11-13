/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <map>
#include <memory>
#include <cinchtest.h>

#include "../hexahedron_definition.h"
#include "../tetrahedron_definition.h"

using def_t = std::shared_ptr<flexi::entity_definition_t>;
using def_map_t = std::map<size_t, def_t>;

TEST(definition, hexahedron) {
  def_t ed(new flexi::hexahedron_definition_t);

  ASSERT_EQ(ed->sub_entities(0), 8);
  ASSERT_EQ(ed->sub_entities(1), 12);
  ASSERT_EQ(ed->sub_entities(2), 6);
} // TEST

TEST(definition, tetrahedron) {
  def_t ed(new flexi::tetrahedron_definition_t);

  ASSERT_EQ(ed->sub_entities(0), 4);
  ASSERT_EQ(ed->sub_entities(1), 6);
  ASSERT_EQ(ed->sub_entities(2), 4);
} // TEST

TEST(definition, select) {
  def_map_t ed;
  ed[4].reset(new flexi::tetrahedron_definition_t);
  ed[8].reset(new flexi::hexahedron_definition_t);
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
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
