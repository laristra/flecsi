#define __FLECSI_PRIVATE__

#include <iostream>
#include <memory>

#include "flecsi/data.hh"
#include "flecsi/topo/structured/interface.hh"
#include "flecsi/topo/structured/simple_box_colorer.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;
using namespace flecsi::topo;
using namespace flecsi::topo::structured_impl;

struct test_mesh : topo::specialization<structured, test_mesh> {
  static constexpr size_t num_dimensions = 1;

  static coloring color(size_t grid_size[num_dimensions],
    size_t nghost_layers,
    size_t ndomain_layers,
    size_t thru_dim,
    size_t ncolors[num_dimensions]) {

    // Create a colorer instance to generate the coloring/partition
    // of the mesh.
    simple_box_colorer<num_dimensions> colorer;

    // Create the coloring info for cells
    auto colored_cells = colorer.color(
      grid_size, nghost_layers, ndomain_layers, thru_dim, ncolors);

    return colored_cells;
  } // color
};

test_mesh::slot tmesh;
test_mesh::cslot coloring;

int
topo_driver() {
  UNIT {
    // Define bounds of a structured mesh
    size_t grid_size[1] = {6};
    size_t nhalo = 1;
    size_t nhalo_domain = 2;
    size_t thru_dim = 0;
    size_t ncolors[1] = {2};

    coloring.allocate(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);
    tmesh.allocate(coloring.get());

    auto colored_ents = coloring.get();
    int owner = colored_ents.exclusive[0].colors[0];
    int dim = colored_ents.mesh_dim;

    std::string fname =
      "smesh_" + std::to_string(dim) + "d_" + std::to_string(owner) + ".txt";

    UNIT_CAPTURE() << "MESH DIM " << dim << std::endl;

    // Print colored ents: overlay, exclusive, shared, ghost, domain-halos
    int edim = colored_ents.entity_dim;
    size_t de_nboxes = colored_ents.num_boxes;
    auto de_ebox = colored_ents.exclusive;
    auto de_obox = colored_ents.overlay;
    auto de_strides = colored_ents.strides;

    UNIT_CAPTURE() << "ENTITY OF DIM " << edim << " COLORING" << std::endl;

    UNIT_CAPTURE() << "   ----->Overlay:\n";
    for(size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << "          dim " << i << " : "
                       << de_obox[n].lowerbnd[i] << ", "
                       << de_obox[n].upperbnd[i] << std::endl;
    }

    UNIT_CAPTURE() << "   ----->Strides:\n";
    for(size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << "           dim " << i << " : " << de_strides[n][i]
                       << std::endl;
    }

    UNIT_CAPTURE() << "   ----->Exclusive:\n";
    for(size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << "          dim " << i << " : "
                       << de_ebox[n].domain.lowerbnd[i] << ", "
                       << de_ebox[n].domain.upperbnd[i] << std::endl;
    }
    UNIT_WRITE(fname);
    UNIT_EQUAL_BLESSED(fname.c_str());
  };
} // ntree_driver

flecsi::unit::driver<topo_driver> driver;
