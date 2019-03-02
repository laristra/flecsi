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

#include <flecsi/utils/ftest.h>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.h>

using namespace flecsi::execution;

flog_register_tag(color);

/*
  Test the raw context interface for gettting color information from
  the FleCSI runtime.
 */

void color_raw(int argc, char ** argv) {
  FTEST();

  auto color = context_t::instance().color();
  auto colors = context_t::instance().colors();

  {
    flog_tag_guard(color);
    flog(info) << "color(raw): " << color << std::endl <<
      "colors(raw): " << colors << std::endl;
  }
  
  ASSERT_EQ(colors, 4);
  ASSERT_LT(color, colors);
}

ftest_register_test(color_raw);

/*
  Test the macro interface for gettting color information from
  the FleCSI runtime.
 */

void color_ui(int argc, char ** argv) {
  FTEST();

  auto color = flecsi_color();
  auto colors = flecsi_colors();

  {
    flog_tag_guard(color);
    flog(info) << "color(macro): " << color << std::endl <<
      "colors(macro): " << colors << std::endl;
  }
  
  ASSERT_EQ(colors,4);
  ASSERT_LT(color, colors);
}

ftest_register_test(color_ui);
