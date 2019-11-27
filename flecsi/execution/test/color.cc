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

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution.hh>

flog_register_tag(color);

/*
  Test the raw context interface for gettting color information from
  the FleCSI runtime.
 */

int
color_raw(int, char **) {

  FTEST();

  auto & c = flecsi::runtime::context_t::instance();
  ASSERT_EQ(c.task_depth(), 1u);
  auto process = c.process();
  auto processes = c.processes();
  auto tpp = c.threads_per_process();

  {
    flog_tag_guard(color);
    flog(info) << "color(raw): " << process << std::endl
               << "colors(raw): " << processes << std::endl
               << "threads_per_process(raw): " << tpp << std::endl;
  }

  ASSERT_EQ(processes, 4u);
  ASSERT_LT(process, processes);

  return FTEST_RESULT();
}

ftest_register_driver(color_raw);

#if 0
/*
  Test the macro interface for gettting color information from
  the FleCSI runtime.
 */

int
color_ui(int, char **) {

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
