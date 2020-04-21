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

#include <flecsi/execution.hh>
#include <flecsi/utils/ftest.hh>

using namespace flecsi;

flog::devel_tag color_tag("color");

/*
  Test the raw context interface for gettting color information from
  the FleCSI runtime.
 */

int
color_raw(int, char **) {
  FTEST {
    auto & c = runtime::context_t::instance();
    flog(info) << "task depth: " << c.task_depth() << std::endl;
    ASSERT_EQ(c.task_depth(), 0u);

    auto process = c.process();
    auto processes = c.processes();
    auto tpp = c.threads_per_process();

    {
      flog::devel_guard guard(color_tag);
      flog(info) << "(raw)" << std::endl
                 << "\tprocess: " << process << std::endl
                 << "\tprocesses: " << processes << std::endl
                 << "\tthreads_per_process: " << tpp << std::endl;
    }

    ASSERT_EQ(processes, 4u);
    ASSERT_LT(process, processes);
  };
}

ftest_register_driver(color_raw);

#if 0
/*
  Test the macro interface for gettting color information from
  the FleCSI runtime.
 */

int
color_ui(int, char **) {
  FTEST {
  auto color = flecsi_color();
  auto colors = flecsi_colors();

  {
    flog::devel_guard guard(color_tag);
    flog(info) << "color(macro): " << color << std::endl
               << "colors(macro): " << colors << std::endl;
  }

  ASSERT_EQ(colors, 4);
  ASSERT_LT(color, colors);
  };
}

ftest_register_driver(color_ui);
#endif
