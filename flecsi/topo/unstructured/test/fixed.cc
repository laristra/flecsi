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
#include "fixed.hh"

#define __FLECSI_PRIVATE__
#include "flecsi/data.hh"
#include "flecsi/execution.hh"
#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

struct unstructured : topo::specialization<topo::unstructured, unstructured> {

  /*--------------------------------------------------------------------------*
    Structure
   *--------------------------------------------------------------------------*/

  enum index_space { vertices, edges, cells };
  using index_spaces = has<cells, vertices, edges>;
  using connectivities = list<from<cells, to<vertices, edges>>,
    from<vertices, to<cells>>,
    from<edges, to<cells>>>;

  enum entity_list { dirichlet, neumann, special_vertices };
  using entity_lists = list<entity<edges, has<dirichlet, neumann>>,
    entity<vertices, has<special_vertices>>>;

  /*--------------------------------------------------------------------------*
    Interface
   *--------------------------------------------------------------------------*/

  template<class B>
  struct interface : B {

    auto cells() {
      return B::template entities<index_space::cells>();
    }

    template<index_space From>
    auto cells(topo::id<From> from) {
      return B::template entities<index_space::cells>(from);
    }

    auto vertices() {
      return B::template entities<index_space::vertices>();
    }

    auto edges() {
      return B::template entities<index_space::edges>();
    }

    template<entity_list List>
    auto edges() {
      return B::template special_entities<unstructured::edges, List>();
    }

  }; // struct interface

  /*--------------------------------------------------------------------------*
    Coloring
   *--------------------------------------------------------------------------*/

  static coloring color() {
    return {};
  } // color

  /*--------------------------------------------------------------------------*
    Initialization
   *--------------------------------------------------------------------------*/

  static void initialize(data::topology_slot<unstructured> & s) {
    (void)s;
  } // initialize

}; // struct unstructured

unstructured::slot mesh;
unstructured::cslot coloring;

int
fixed_driver() {
  UNIT {
    flog(info) << "Indices: " << connectivity[process()][0].indices.size()
               << std::endl;
  };
} // unstructured_driver

flecsi::unit::driver<fixed_driver> driver;
