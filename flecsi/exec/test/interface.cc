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

#define __FLECSI_PRIVATE__
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;
using namespace flecsi::exec;

int index_task(launch_domain) {
  UNIT {
    flog(info) << "program: " << program() << std::endl;
    flog(info) << "processes: " << processes() << std::endl;
    flog(info) << "process: " << process() << std::endl;
    //flog(info) << "threads per process: " << threads_per_process() << std::endl;
    //flog(info) << "threads: " << threads() << std::endl;
    //flog(info) << "colors: " << colors() << std::endl;
    //flog(info) << "color: " << color() << std::endl;

    ASSERT_LT(process(), processes());
    ASSERT_GE(process(), 0u);
    //ASSERT_LT(color(), domain.size());
    //ASSERT_GE(color(), 0u);
    //ASSERT_EQ(colors(), domain.size());
  };
}

int
interface_driver() {
  UNIT {
    ASSERT_EQ(test<index_task>(launch_domain{processes()+4}), 0);
    int task_return = test<index_task>(launch_domain{processes()+4});
    ASSERT_EQ(task_return, 0);
  };
} // interface_driver

unit::driver<interface_driver> driver;
