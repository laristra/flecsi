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

#include "control.hh"

using namespace flecsi;
using namespace flecsi::topo;

template<typename T>
using single = field<T, data::single>;
const single<double>::definition<global> lue;

void
init(double v, single<double>::accessor<wo> gv) {
  gv = v;
}

void
print(single<double>::accessor<ro> gv) {
  flog(trace) << "global value: " << gv << std::endl;
}

int
advance() {

  const auto v = lue(global_topology);
  execute<init>(42.0, v);
  execute<print>(v);

  return 0;
}
control::action<advance, cp::advance> advance_action;
