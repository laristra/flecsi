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

  enum entity_list { dirichlet, neumann, special_vertices };
  using entity_lists = list<entity<edges, has<dirichlet, neumann>>,
    entity<vertices, has<special_vertices>>>;

  static coloring color(std::string const & filename) {
    topo::unstructured_impl::simple_definition definition(filename.c_str());
    return {processes(), {}, {}};
  } // color
}; // struct unstructured

unstructured::slot mesh;
unstructured::cslot coloring;

void
allocate(topo::resize::Field::accessor<wo> a) {
  a = data::partition::make_row(color(), 2);
}
void
init(field<util::id, data::ragged>::mutator m) {
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
    coloring.allocate("simple2d-8x8.msh");
    // Provide 0s to allow vacuous topology construction:
    coloring.get().index_colorings.resize(unstructured::index_spaces::size);
    mesh.allocate(coloring.get());
    auto & neuf =
      mesh->special.get<unstructured::edges>().get<unstructured::neumann>();
    auto & p = mesh->meta.ragged.get_partition(neuf.fid);
    execute<allocate>(p.sizes());
    p.resize();
    execute<init>(neuf(mesh->meta));
    EXPECT_EQ(test<check>(mesh), 0);
  };
} // unstructured_driver

flecsi::unit::driver<unstructured_driver> driver;
