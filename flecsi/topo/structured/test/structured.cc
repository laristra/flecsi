#define __FLECSI_PRIVATE__

#include <iostream>
#include <memory>

#include "flecsi/data.hh"
#include "flecsi/topo/structured/box.hh"
#include "flecsi/topo/structured/dependent_entities_colorer.hh"
#include "flecsi/topo/structured/interface.hh"
#include "flecsi/topo/structured/simple_box_colorer.hh"
#include "flecsi/topo/structured/test/test_utils.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;
using namespace flecsi::topo;
using namespace flecsi::topo::structured_impl;

/* 1D Mesh  */
struct test_mesh_1d : specialization<structured, test_mesh_1d> {
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

test_mesh_1d::slot tmesh_1d;
test_mesh_1d::cslot coloring_1d;

int
topo_driver_1d() {
  UNIT {
    // Define bounds of a structured mesh
    std::size_t grid_size[1] = {6};
    std::size_t nhalo = 1;
    std::size_t nhalo_domain = 2;
    std::size_t thru_dim = 0;
    std::size_t ncolors[1] = {2};

    coloring_1d.allocate(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);
    tmesh_1d.allocate(coloring_1d.get());

    auto & colored_ents = coloring_1d.get();
    int owner = colored_ents.exclusive[0].colors[0];
    int dim = colored_ents.mesh_dim;

    auto & overlay = colored_ents.overlay[0];
    std::vector<std::size_t> local_upbnd(dim);

    for(int i = 0; i < dim; ++i)
      local_upbnd[i] = overlay.upperbnd[i] - overlay.lowerbnd[i];

    box<1> lbox(local_upbnd);

    using id_array = box<1>::id_array;
    id_array indices;

    for(auto c : lbox) {
      lbox.indices_from_offset(c, indices);
      auto offset = lbox.offset_from_indices(indices);
      EXPECT_EQ(c, offset);
      UNIT_CAPTURE() << "Cell id " << c << " with offset " << offset
                     << " and indices = [" << indices[0] << "]\n";
    }

    std::string fname = "smesh_" + std::to_string(dim) + "d_" +
                        std::to_string(owner) + ".current";

    print_part_primary_entity(colored_ents);
    UNIT_EQUAL_BLESSED(fname.c_str());
  };
} // topo_driver

/* 2D Mesh  */
struct test_mesh_2d : specialization<structured, test_mesh_2d> {
  static constexpr std::size_t num_dimensions = 2;

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
test_mesh_2d::slot tmesh_2d;
test_mesh_2d::cslot coloring_2d;

int
topo_driver_2d() {
  UNIT {
    // Define bounds of a structured mesh
    std::size_t grid_size[2] = {6, 6};
    std::size_t nhalo = 1;
    std::size_t nhalo_domain = 2;
    std::size_t thru_dim = 0;
    std::size_t ncolors[2] = {2, 1};

    coloring_2d.allocate(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);
    tmesh_2d.allocate(coloring_2d.get());

    auto & colored_ents = coloring_2d.get();
    int owner = colored_ents.exclusive[0].colors[0];
    int dim = colored_ents.mesh_dim;

    auto & overlay = colored_ents.overlay[0];
    std::vector<std::size_t> local_upbnd(dim);

    for(int i = 0; i < dim; ++i)
      local_upbnd[i] = overlay.upperbnd[i] - overlay.lowerbnd[i];

    box<2> lbox(local_upbnd);

    using id_array = box<2>::id_array;
    id_array indices;

    for(auto c : lbox) {
      lbox.indices_from_offset(c, indices);
      auto offset = lbox.offset_from_indices(indices);
      EXPECT_EQ(c, offset);
      UNIT_CAPTURE() << "Cell id " << c << " with offset " << offset
                     << " and indices = [" << indices[0] << ", " << indices[1]
                     << "]\n";
    }

    std::string fname = "smesh_" + std::to_string(dim) + "d_" +
                        std::to_string(owner) + ".current";

    print_part_primary_entity(colored_ents);
    UNIT_EQUAL_BLESSED(fname.c_str());
  };
} // topo_driver

/* 3D Mesh  */
struct test_mesh_3d : specialization<structured, test_mesh_3d> {
  static constexpr std::size_t num_dimensions = 3;

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
test_mesh_3d::slot tmesh_3d;
test_mesh_3d::cslot coloring_3d;

int
topo_driver_3d() {
  UNIT {
    // Define bounds of a structured mesh
    std::size_t grid_size[3] = {6, 6, 6};
    std::size_t nhalo = 1;
    std::size_t nhalo_domain = 2;
    std::size_t thru_dim = 0;
    std::size_t ncolors[3] = {2, 1, 1};

    coloring_3d.allocate(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);
    tmesh_3d.allocate(coloring_3d.get());

    auto & colored_ents = coloring_3d.get();
    int owner = colored_ents.exclusive[0].colors[0];
    int dim = colored_ents.mesh_dim;

    auto & overlay = colored_ents.overlay[0];
    std::vector<std::size_t> local_upbnd(dim);

    for(int i = 0; i < dim; ++i)
      local_upbnd[i] = overlay.upperbnd[i] - overlay.lowerbnd[i];

    box<3> lbox(local_upbnd);

    using id_array = box<3>::id_array;
    id_array indices;

    for(auto c : lbox) {
      lbox.indices_from_offset(c, indices);
      auto offset = lbox.offset_from_indices(indices);
      EXPECT_EQ(c, offset);
      UNIT_CAPTURE() << "Cell id " << c << " with offset " << offset
                     << " and indices = [" << indices[0] << ", " << indices[1]
                     << ", " << indices[2] << "]\n";
    }

    std::string fname = "smesh_" + std::to_string(dim) + "d_" +
                        std::to_string(owner) + ".current";

    print_part_primary_entity(colored_ents);
    UNIT_EQUAL_BLESSED(fname.c_str());
  };
} // topo_driver

flecsi::unit::driver<topo_driver_1d> driver_1d;
flecsi::unit::driver<topo_driver_2d> driver_2d;
flecsi::unit::driver<topo_driver_3d> driver_3d;
