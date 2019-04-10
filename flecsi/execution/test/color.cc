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

int
color_raw(int argc, char ** argv) {

  FTEST();

  auto depth = context_t::instance().task_depth();
  auto rank = context_t::instance().rank();
  auto size = context_t::instance().size();
  auto cpr = context_t::instance().colors_per_rank();

  {
    flog_tag_guard(color);
    flog(info) << "color(raw): " << rank << std::endl
               << "colors(raw): " << size << std::endl
               << "colors_per_rank(raw): " << cpr << std::endl;
  }

  ASSERT_EQ(size, 4);
  ASSERT_LT(rank, size);

  return 0;
}

ftest_register_test(color_raw);

#if 0
/*
  Test the macro interface for gettting color information from
  the FleCSI runtime.
 */

int
color_ui(int argc, char ** argv) {

  FTEST();

  auto color = flecsi_color();
  auto colors = flecsi_colors();

  {
    flog_tag_guard(color);
    flog(info) << "color(macro): " << color << std::endl
               << "colors(macro): " << colors << std::endl;
  }

  ASSERT_EQ(colors, 4);
  ASSERT_LT(color, colors);

  return 0;
}

ftest_register_test(color_ui);
#endif
