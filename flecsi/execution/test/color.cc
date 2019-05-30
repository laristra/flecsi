/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
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
  auto process = context_t::instance().process();
  auto processes = context_t::instance().processes();
  auto tpp = context_t::instance().threads_per_process();

  {
    flog_tag_guard(color);
    flog(info) << "color(raw): " << process << std::endl
               << "colors(raw): " << processes << std::endl
               << "threads_per_process(raw): " << tpp << std::endl;
  }

  ASSERT_EQ(processes, 4);
  ASSERT_LT(process, processes);

  return 0;
}

ftest_register_driver(color_raw);

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

ftest_register_driver(color_ui);
#endif
