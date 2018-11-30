/*----------------------------------------------------------------------------*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *----------------------------------------------------------------------------*/

#include <flecsi/utils/demangle.h>
#include <flecsi/utils/test/print_type.h>

#include <cinchtest.h>

TEST(common, all) {

  // demangle, type
  // The results depend on #ifdef __GNUG__, so we'll just exercise
  // these functions, without checking for particular results.
  EXPECT_NE(flecsi::utils::demangle("foo"), "");
  const std::string str_demangle = flecsi::utils::demangle(typeid(int).name()),
                    str_type = flecsi::utils::type<int>();
  EXPECT_NE(str_demangle, "");
  EXPECT_NE(str_type, "");
  EXPECT_EQ(str_demangle, str_type);

} // TEST

/*----------------------------------------------------------------------------*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *----------------------------------------------------------------------------*/
