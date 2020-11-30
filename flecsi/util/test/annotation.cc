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

#include <chrono>
#include <thread>

#include <caliper/RegionProfile.h>

#include "flecsi/util/annotation.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

std::size_t
check_custom(double val) {
  return val * 100;
}

struct test_context : util::annotation::context<test_context> {
  static constexpr char name[] = "test-context";
};

void
wait() {
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

int
annotation_driver() {
  UNIT {
    namespace ann = flecsi::util::annotation;

    auto & c = run::context::instance();
    auto rank = c.process();
    auto size = c.processes();

    cali::RegionProfile rp;
    rp.start();

    { // test custom annotation context/region
      {
        ann::guard<test_context, ann::detail::low> g("custom");
        std::this_thread::sleep_for(std::chrono::milliseconds(10 * (rank + 1)));
      }

      auto times = std::get<0>(rp.exclusive_region_times(test_context::name));

      auto custom_time = times.find("custom");
      EXPECT_NE(custom_time, times.end());
      auto combined_time =
        reduce<check_custom, exec::fold::sum<std::size_t>, mpi>(times["custom"])
          .get();
      EXPECT_EQ(combined_time, size * (size + 1) / 2);
    }

    { // test user execution
      execute<wait, mpi>();
      auto times = std::get<0>(rp.exclusive_region_times(ann::execution::name));
      auto wait_time =
        times.find(ann::execute_task_user::name + "->" + util::symbol<wait>());
      EXPECT_NE(wait_time, times.end());
      EXPECT_EQ((static_cast<int>((*wait_time).second * 100)), 4);
    }
  };
} // annotation_driver

unit::driver<annotation_driver> driver;
