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

#include <flecsi/execution.hh>
#include <flecsi/util/annotation.hh>

using namespace flecsi;
using namespace flecsi::util;

struct user_execution : annotation::context<user_execution> {
  static constexpr char name[] = "User-Execution";
};

struct main_region : annotation::region<annotation::execution> {
  inline static const std::string name{"main"};
};

struct sleeper_region : annotation::region<user_execution> {
  inline static const std::string name{"sleeper"};
};

struct sleeper_subtask : annotation::region<user_execution> {
  inline static const std::string name{"subtask"};
  static constexpr annotation::detail detail_level = annotation::detail::high;
};

void
sleeper() {
  annotation::rguard<sleeper_region> guard;

  annotation::begin<sleeper_subtask>();
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  annotation::end<sleeper_subtask>();

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

int
top_level_action() {

  sleeper();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  return 0;
}

int
main(int argc, char * argv[]) {
  annotation::rguard<main_region> main_guard;

  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  }

  status = (annotation::guard<annotation::execution, annotation::detail::low>(
              "top-level-task"),
    flecsi::start(top_level_action));

  flecsi::finalize();

  return status;
} // main
