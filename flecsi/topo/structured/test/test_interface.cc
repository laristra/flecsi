#define __FLECSI_PRIVATE__

#include <iostream>
#include <memory>

#include "flecsi/data.hh"
#include "flecsi/topo/structured/dependent_entities_colorer.hh"
#include "flecsi/topo/structured/interface.hh"
#include "flecsi/topo/structured/simple_box_colorer.hh"
#include "flecsi/topo/structured/test/test_utils.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;
using namespace flecsi::topo;
using namespace flecsi::topo::structured_impl;

struct test_mesh : specialization<structured, test_mesh> {
  static constexpr std::size_t num_dimensions = 1;

  static coloring color(std::size_t grid_size[num_dimensions],
    std::size_t nghost_layers,
    std::size_t ndomain_layers,
    std::size_t thru_dim,
    std::size_t ncolors[num_dimensions]) {

    // Create the coloring info for cells
    auto cells_part = simple_box_colorer<num_dimensions>(
      grid_size, nghost_layers, ndomain_layers, thru_dim, ncolors);

    auto depent_part = dependent_entities_colorer(cells_part);

    return cells_part;
  } // color
};

test_mesh::slot tmesh;
test_mesh::cslot coloring;

int
topo_driver() {
  UNIT {
    // Define bounds of a structured mesh
    std::size_t grid_size[1] = {6};
    std::size_t nhalo = 1;
    std::size_t nhalo_domain = 2;
    std::size_t thru_dim = 0;
    std::size_t ncolors[1] = {2};

    coloring.allocate(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);
    tmesh.allocate(coloring.get());

    auto & colored_ents = coloring.get();
    int owner = colored_ents.exclusive[0].colors[0];
    int dim = colored_ents.mesh_dim;

    std::string fname = "smesh_" + std::to_string(dim) + "d_" +
                        std::to_string(owner) + ".current";

    print_part_primary_entity(colored_ents);
    UNIT_EQUAL_BLESSED(fname.c_str());
  };
} // topo_driver

flecsi::unit::driver<topo_driver> driver;
