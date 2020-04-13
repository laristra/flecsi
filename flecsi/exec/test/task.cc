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

#include "flecsi/util/ftest.hh"

#define __FLECSI_PRIVATE__
#include <flecsi/execution.hh>

using namespace flecsi;

log::devel_tag task_tag("task");

namespace hydro {

template<typename TYPE>
void
simple(TYPE arg) {
  log::devel_guard guard(task_tag);
  flog(info) << "arg(" << arg << ")\n";
} // simple

template<class T>
void
seq(const T & s) {
  log::devel_guard guard(task_tag);
  [&s](auto && log) {
    bool first = true;
    for(auto & x : s) {
      if(first)
        first = false;
      else
        log << ',';
      log << x;
    }
    log << ")\n";
  }(flog_info("s(")); // keep temporary alive throughout
}

void
mpi(int * p) {
  *p = 1;
}

} // namespace hydro

int
test_driver(int, char **) {
  FTEST {
    execute<hydro::simple<float>>(6.2);
    execute<hydro::simple<double>>(5.3);
    execute<hydro::simple<const float &>>(4.4);
    execute<hydro::simple<const double &>>(3.5);
    using V = std::vector<std::string>;
    execute<hydro::seq<V>>(V{"Elementary", " Dear Data"});

    int x = 0;
    execute<hydro::mpi, flecsi::index, mpi>(&x);
    ASSERT_EQ(x, 1); // NB: MPI calls are synchronous
  };
}

ftest_register_driver(test_driver);
