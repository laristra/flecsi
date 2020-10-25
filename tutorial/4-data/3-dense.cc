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

#include <flecsi/data.hh>
#include <flecsi/execution.hh>
#include <flecsi/flog.hh>

#include "canonical.hh"
#include "control.hh"

using namespace flecsi;

canon::slot canonical;
canon::cslot coloring;

template<typename T>
using dense_field = field<T, data::dense>;

const dense_field<double>::definition<canon, canon::cells> pressure;

void
init(canon::accessor<wo> t, dense_field<double>::accessor<wo> p) {
  std::size_t off{0};
  for(const auto c : t.entities<canon::cells>()) {
    p[c] = double(off++) * 2.0;
  } // for
} // init

void
print(canon::accessor<ro> t, dense_field<double>::accessor<ro> p) {
  std::size_t off{0};
  for(auto c : t.entities<canon::cells>()) {
    flog(info) << "cell " << off++ << " has pressure " << p[c] << std::endl;
  } // for
} // print

int
advance() {
  coloring.allocate("test.txt");
  canonical.allocate(coloring.get());

  auto pf = pressure(canonical);
#if 1
  execute<init>(canonical, pf);
  execute<print>(canonical, pf);
#endif

  return 0;
}
control::action<advance, cp::advance> advance_action;
