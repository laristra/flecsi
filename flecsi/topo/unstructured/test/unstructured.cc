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
#include "flecsi/topo/unstructured/test/simple_definition.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>

using namespace flecsi;

struct unstructured : topo::specialization<topo::unstructured, unstructured> {
  enum index_space { vertices, edges, cells };
  using index_spaces = has<cells, vertices, edges>;
  using connectivities = list<entity<cells, has<vertices, edges>>>;

#if 0
  enum range { dirichlet, neumann, special_vertices };
  using iterators = util::types<entity<edges, iterates<dirichlet>, entity<edges, iterates<neumann>>,
    entity<vertices, iterates<special_vertices>>>;
#endif
  /*
    DAVIS: I don't know if this is what you had in mind, but I could see this
    type of interface being intuitive for users. It would be used like:

    for(auto e: m.edges(dirichlet)) {
      ...
    }
   */

  static coloring color(std::string const & filename) {
    topo::unstructured_impl::simple_definition definition(filename.c_str());
    return {};
  } // color
}; // struct unstructured

unstructured::slot mesh;
unstructured::cslot coloring;

int
unstructured_driver() {
  UNIT { coloring.allocate("simple2d-8x8.msh"); };
} // unstructured_driver

flecsi::unit::driver<unstructured_driver> driver;
