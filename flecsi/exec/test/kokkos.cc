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
#include <flecsi/data.hh>

#include "flecsi/topo/canonical/interface.hh"

#include "flecsi/util/unit.hh"
#include <flecsi/data/accessor.hh>
#include <flecsi/exec/kernel_interface.hh>

#include <Kokkos_Core.hpp>

using namespace flecsi;

struct canon : topo::specialization<topo::canonical, canon> {
  enum index_space { vertices, cells };
  using index_spaces = has<cells, vertices>;
  using connectivities = util::types<from<cells, has<vertices>>>;

  static coloring color(std::string const &) {
    return {{40, 60}, 2};
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::cells> cell_field;
auto pressure = cell_field(canonical);

const int pvalue = 35;

int
init(canon::accessor<wo> t, field<double>::accessor<wo> c) {
  UNIT {
    flecsi::exec::parallel_for(
      c.span(), KOKKOS_LAMBDA(auto & cv) { cv = pvalue; }, std::string("test"));
  };
} // init

int
local_kokkos(field<double>::accessor<rw> c) {
  UNIT {
    // Parallel for
    flecsi::exec::parallel_for(
      c.span(),
      KOKKOS_LAMBDA(auto cv) { assert(cv == pvalue); },
      std::string("pfor1"));

    forall(cv, c.span(), "pfor2") {
      assert(cv == pvalue);
    }; // forall

    // Reduction
    std::size_t res = 0;
    flecsi::exec::parallel_reduce(
      c.span(),
      KOKKOS_LAMBDA(auto cv, std::size_t & up) { up += cv; },
      flecsi::exec::reducer::sum<std::size_t>(res),
      std::string("pred1"));
    assert(pvalue * c.span().size() == res);
    res = 0;
    reduceall(
      cv, up, c.span(), flecsi::exec::reducer::sum<std::size_t>(res), "pred2") {
      up += cv;
    };
    assert(pvalue * c.span().size() == res);
  };
}

int
kokkos_driver() {
  UNIT {

    Kokkos::print_configuration(std::cerr);

    // use canonical
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());
    EXPECT_EQ(test<init>(canonical, pressure), 0);

    EXPECT_EQ(test<local_kokkos>(pressure), 0);
  };
} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

flecsi::unit::driver<kokkos_driver> driver;
