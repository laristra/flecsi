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
#include "flecsi/data.hh"
#include "flecsi/execution.hh"
#include "flecsi/topo/unstructured/coloring_utils.hh"
#include "flecsi/topo/unstructured/interface.hh"
//#include "flecsi/topo/unstructured/mpi_communicator.hh"
#include "flecsi/topo/unstructured/test/simple_definition.hh"
#include "flecsi/util/parmetis.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

struct unstructured : topo::specialization<topo::unstructured, unstructured> {

  /*--------------------------------------------------------------------------*
    Structure
   *--------------------------------------------------------------------------*/

  enum index_space { vertices, edges, cells };
  using index_spaces = has<cells, vertices, edges>;
  using connectivities = list<from<cells, to<vertices, edges>>>;

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

#if 0
  struct coloring_policy {
    // primary independent closure token
    using primary =
      topo::unstructured_impl::primary_independent<index_space::cells,
        2 /* dimension */,
        0 /* through dimension */,
        1 /* depth */>;

    using auxiliary = std::tuple<
      topo::unstructured_impl::auxiliary_independent<index_space::vertices,
        0 /* dimension */,
        2 /* primary dimension */>,
      topo::unstructured_impl::auxiliary_independent<index_space::edges,
        1 /* dimension */,
        2 /* primary dimension */>>;

    static constexpr size_t auxiliary_colorings =
      std::tuple_size<auxiliary>::value;
    using definition = topo::unstructured_impl::simple_definition;
    using communicator = topo::unstructured_impl::mpi_communicator;
  }; // struct coloring_policy
#endif

  static coloring color(std::string const & filename) {
    (void)filename;
    topo::unstructured_impl::simple_definition sd(filename.c_str());
    const size_t colors{processes()};
    auto [naive, cells, v2c, c2c] = topo::unstructured_impl::make_dcrs(sd, 1);
    auto raw = util::parmetis::color(naive, colors);
    auto coloring = topo::unstructured_impl::distribute(naive, colors, raw);
#if 0
    auto closure = topo::unstructured_impl::closure<coloring_policy>(
      sd, coloring[0], MPI_COMM_WORLD);

    // FIXME: dummy information so that tests pass
    closure.connectivity_sizes.push_back({10, 10});
    return closure;
#endif
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

void
init(topo::connect_field::mutator<rw> m) {
  m[0].resize(2, 47);
}

int
check(unstructured::accessor<ro> t) {
  UNIT {
    for(auto i :
      t.special_entities<unstructured::edges, unstructured::neumann>()) {
      static_assert(std::is_same_v<decltype(i), topo::id<unstructured::edges>>);
      EXPECT_EQ(i, 47u);
    }
  };
}

int
unstructured_driver() {
  UNIT {
#if 0
    coloring.allocate("simple2d-8x8.msh");
    mesh.allocate(coloring.get());

    auto & neuf =
      mesh->special_.get<unstructured::edges>().get<unstructured::neumann>();
    execute<init>(neuf(mesh->meta));
    EXPECT_EQ(test<check>(mesh), 0);
#endif
  };
} // unstructured_driver

flecsi::unit::driver<unstructured_driver> driver;
