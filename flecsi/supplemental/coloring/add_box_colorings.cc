/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <cinchlog.h>
#include <mpi.h>

#include <flecsi/execution/execution.h>
#include <flecsi/coloring/simple_box_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/supplemental/coloring/add_box_colorings.h>

clog_register_tag(coloring);
clog_register_tag(coloring_output);

namespace flecsi {
namespace execution {

void add_box_colorings(coloring_map_t map) {

  clog_set_output_rank(0);

  // Get the context instance.
  context_t & context_ = context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  {
    clog_tag_guard(coloring);
    clog(info) << "add_colorings, rank: " << rank << std::endl;
  }

  // Define bounds for the box corresponding to cells 
  size_t grid_size[2] = {10,10};
  size_t ncolors[2]={2,1};
  size_t nhalo = 1;
  size_t nhalo_domain = 0; 
  size_t thru_dim = 0; 

  flecsi::coloring::box_coloring_t colored_cells;
  flecsi::coloring::coloring_info_t colored_cells_aggregate;

  // Create a colorer instance to generate the coloring.
  auto colorer = std::make_shared<flecsi::coloring::simple_box_colorer_t<2>>();

  //Create the coloring info for cells
  colored_cells = colorer->color(grid_size, nhalo, nhalo_domain, thru_dim, 
                                 ncolors);

  //Create the aggregate type for cells
  colored_cells_aggregate = colorer->create_aggregate_color_info(colored_cells);

   // Create a communicator instance to get coloring_info_t from other ranks 
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->gather_coloring_info(colored_cells_aggregate);

  //Add box coloring to context
  context_.add_box_coloring(map.cells, colored_cells, cell_coloring_info);

} // add_box_colorings

flecsi_register_mpi_task(add_box_colorings, flecsi::execution);

} // namespace execution
} // namespace flecsi
