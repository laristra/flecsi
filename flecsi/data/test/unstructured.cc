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
#include "flecsi/topo/unstructured/coloring_utils.hh"
#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/topo/unstructured/simple_definition.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;

struct unstructured : topo::specialization<topo::unstructured, unstructured> {
  enum index_space { vertices, cells };
  using index_spaces = has<cells, vertices>;
  using connectivities = util::types<from<cells, has<vertices>>>;

  static coloring color(std::string const & filename) {
    topo::unstructured_impl::simple_definition definition(filename.c_str());
    return topo::unstructured_impl::color(&definition, processes());
  } // color
}; // struct unstructured

unstructured::slot mesh;
unstructured::cslot coloring;

int
unstructured_driver() {
  UNIT { coloring.allocate("simple2d-8x8.msh"); };
} // unstructured_driver

flecsi::unit::driver<unstructured_driver> driver;
