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

template<typename T>
using single = field<T, data::single>;
const single<std::size_t>::definition<topo::index> lue;

inline topo::index::slot custom_topology;

void
init(single<std::size_t>::accessor<wo> iv) {
  flog(trace) << "initializing value on color " << color() << " of " << colors()
              << std::endl;
  iv = color();
}

void
print(single<std::size_t>::accessor<ro> iv) {
  flog(trace) << "index value: " << iv << " (color " << color() << " of "
              << colors() << ")" << std::endl;
}

int
advance() {

  custom_topology.allocate(4);

  execute<init>(lue(process_topology));
  execute<print>(lue(process_topology));

  execute<init>(lue(custom_topology));
  execute<print>(lue(custom_topology));

  return 0;
}
control::action<advance, cp::advance> advance_action;
