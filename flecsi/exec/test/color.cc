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

#include "flecsi/util/unit.hh"
#include <flecsi/execution.hh>

using namespace flecsi;

log::devel_tag color_tag("color");

/*
  Test the raw context interface for gettting color information from
  the FleCSI runtime.
 */

int
color_raw() {
  UNIT {
    auto & c = run::context::instance();
    flog(info) << "task depth: " << c.task_depth() << std::endl;
    ASSERT_EQ(c.task_depth(), 0u);

    auto process = c.process();
    auto processes = c.processes();
    auto tpp = c.threads_per_process();

    {
      log::devel_guard guard(color_tag);
      flog(info) << "(raw)" << std::endl
                 << "\tprocess: " << process << std::endl
                 << "\tprocesses: " << processes << std::endl
                 << "\tthreads_per_process: " << tpp << std::endl;
    }

    ASSERT_EQ(processes, 4u);
    ASSERT_LT(process, processes);
  };
} // color_raw

flecsi::unit::driver<color_raw> driver;
