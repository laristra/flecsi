/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \authors jgraham
/// \date Initial file creation: Feb 2, 2017
///

#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/coloring/parmetis_colorer.h"
#include "flecsi/coloring/mpi_communicator.h"
#include "flecsi/supplemental/mesh/test_mesh_2d.h"

#define INDEX_ID 0
#define VERSIONS 1
#define NX 32
#define NY 32
#define CFL 0.5
#define U 1.0
#define V 1.0
#define DX (1.0  / static_cast<double>(NX - 1))
#define DY (1.0  / static_cast<double>(NY - 1))
#define DT std::min(CFL * DX / U, CFL * DY / V)
#define X_ADVECTION U * DT / DX
#define Y_ADVECTION V * DT / DX

using namespace flecsi;
using namespace supplemental;

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = flecsi::data::legion::dense_handle_t<T, EP, SP, GP>;

void initialize_data(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> phi);
flecsi_register_task(initialize_data, loc, single);

void write_to_disk(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        const int my_color);
flecsi_register_task(write_to_disk, loc, single);

void calculate_exclusive_x_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update);
flecsi_register_task(calculate_exclusive_x_update, loc, single);

void advect_owned_cells_in_x(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update);
flecsi_register_task(advect_owned_cells_in_x, loc, single);

void calculate_exclusive_y_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update);
flecsi_register_task(calculate_exclusive_y_update, loc, single);

void advect_owned_cells_in_y(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update);
flecsi_register_task(advect_owned_cells_in_y, loc, single);

flecsi_register_field(test_mesh_2d_t, lax, cell_ID, size_t, dense,
        INDEX_ID, VERSIONS);
flecsi_register_field(test_mesh_2d_t, lax, phi, double, dense,
        INDEX_ID, VERSIONS);
flecsi_register_field(test_mesh_2d_t, lax, phi_update, double, dense,
        INDEX_ID, VERSIONS);

namespace flecsi {
namespace execution {

void add_colorings(int dummy);
flecsi_register_mpi_task(add_colorings);

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {

  flecsi_execute_mpi_task(add_colorings, 0);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();

  const int my_color = runtime->find_local_MPI_rank();

  auto ch = flecsi_get_client_handle(test_mesh_2d_t, meshes, mesh1);

  auto phi_handle =
          flecsi_get_handle(ch, lax, phi, double, dense, INDEX_ID);
  auto phi_update_handle =
          flecsi_get_handle(ch, lax, phi_update, double, dense, INDEX_ID);
  auto global_IDs_handle =
          flecsi_get_handle(ch, lax, cell_ID, size_t, dense, INDEX_ID);

  flecsi_execute_task(initialize_data, single, global_IDs_handle, phi_handle);

  double time = 0.0;
  while(time < 0.165) {
      time += DT;

      if(my_color == 0)
          std::cout << "t=" << time << std::endl;

      flecsi_execute_task(calculate_exclusive_x_update, single,
          global_IDs_handle, phi_handle, phi_update_handle);

      flecsi_execute_task(advect_owned_cells_in_x, single, global_IDs_handle,
          phi_handle, phi_update_handle);

      flecsi_execute_task(calculate_exclusive_y_update, single,
          global_IDs_handle, phi_handle, phi_update_handle);

      flecsi_execute_task(advect_owned_cells_in_y, single, global_IDs_handle,
          phi_handle, phi_update_handle);
  }

  if(my_color == 0)
      std::cout << "time " << time << std::endl;

  flecsi_execute_task(write_to_disk, single, global_IDs_handle, phi_handle,
          my_color);

  if(my_color == 0)
      std::cout << "lax wendroff ... all tasks issued" << std::endl;

} // driver

void add_colorings(int dummy) {

  // Get the context instance.
  context_t & context_ = context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Read the mesh definition from file.
  const size_t M(32), N(32);
  flecsi::io::simple_definition_t sd("simple2d-32x32.msh");

  // Create the dCRS representation for the distributed colorer.
  auto dcrs = flecsi::coloring::make_dcrs(sd);

  // Create a colorer instance to generate the primary coloring.
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();

  // Cells index coloring.
  flecsi::coloring::index_coloring_t cells;
  flecsi::coloring::coloring_info_t cell_color_info;

  // Create the primary coloring.
  cells.primary = colorer->color(dcrs);

  for(auto cell : cells.primary)
    clog(trace) << "rank " << rank << "primary coloring " << cell << std::endl;

  // Compute the dependency closure of the primary cell coloring
  // through vertex intersections (specified by last argument "0").
  // To specify edge or face intersections, use 1 (edges) or 2 (faces).
  auto closure = flecsi::topology::entity_neighbors<2,2,0>(sd, cells.primary);

  for(auto cell : cells.primary) {
    const size_t y_index = cell / M;
    const size_t x_index = cell % M;
    if(x_index == 0)
      closure.insert(y_index * M + M - 1);
    else if(x_index == (M-1))
      closure.insert(y_index * M);
    if(y_index == 0)
      closure.insert((N-1) * M + x_index);
    else if (y_index == (N-1))
      closure.insert(x_index);
  }

  for(auto cell: closure)
    clog(trace) << "rank " << rank  << "closure" << cell << std::endl;

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nearest_neighbors =
    flecsi::utils::set_difference(closure, cells.primary);

  for(auto cell: nearest_neighbors)
    clog(trace) << "rank " << rank  << "nearest neighbors" << cell << std::endl;

  // Create a communicator instance to get neighbor information.
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();

  // Get the intersection of our nearest neighbors with the nearest
  // neighbors of other ranks. This map of sets will only be populated
  // with intersections that are non-empty
  auto closure_intersection_map =
    communicator->get_intersection_info(nearest_neighbors);

  // We can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors. For most mesh types
  // we actually need information about the ownership of these indices
  // so that we can deterministically assign rank ownership to vertices.
  auto nearest_neighbor_closure =
    flecsi::topology::entity_neighbors<2,2,0>(sd, nearest_neighbors);

  for(auto itr: nearest_neighbor_closure)
    clog(trace) << "rank " << rank  << "nearest neighbor closure" <<
    itr << std::endl;

  // Subtracting out the closure leaves just the
  // next nearest neighbors.
  auto next_nearest_neighbors =
    flecsi::utils::set_difference(nearest_neighbor_closure, closure);

  for(auto itr: next_nearest_neighbors)
    clog(trace) << "rank " << rank  << "next nearest neighbor" << itr << std::endl;

  // The union of the nearest and next-nearest neighbors gives us all
  // of the cells that might reference a vertex that we need.
  auto all_neighbors = flecsi::utils::set_union(nearest_neighbors,
    next_nearest_neighbors);

  for(auto itr: all_neighbors)
    clog(trace) << "rank " << rank  << "all neighbors" << itr << std::endl;

  // Get the rank and offset information for our nearest neighbor
  // dependencies. This also gives information about the ranks
  // that access our shared cells.
  auto cell_nn_info =
    communicator->get_primary_info(cells.primary, nearest_neighbors);

  // Get the rank and offset information for all relevant neighbor
  // dependencies. This information will be necessary for determining
  // shared vertices.
  auto cell_all_info =
    communicator->get_primary_info(cells.primary, all_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
  size_t offset(0);
  for(auto i: cells.primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> remote_info_map;
  for(auto i: std::get<1>(cell_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared cell information.
  {
  size_t offset(0);
  for(auto i: std::get<0>(cell_nn_info)) {
    if(i.size()) {
      cells.shared.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));

      // Collect all colors with whom we require communication
      // to send shared information.
      cell_color_info.shared_users = flecsi::utils::set_union(
        cell_color_info.shared_users, i);
    }
    else {
      cells.exclusive.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));
    } // if
    ++offset;
  } // for
  } // scope

  // Populate ghost cell information.
  {
  size_t offset(0);
  for(auto i: std::get<1>(cell_nn_info)) {
    cells.ghost.insert(i);

    // Collect all colors with whom we require communication
    // to receive ghost information.
    cell_color_info.ghost_owners.insert(i.rank);
  } // for
  } // scope

  cell_color_info.exclusive = cells.exclusive.size();
  cell_color_info.shared = cells.shared.size();
  cell_color_info.ghost = cells.ghost.size();

  for(auto itr: cells.exclusive)
    clog(trace) << "rank " << rank  << "exclusive cells " << itr << std::endl;
  for(auto itr: cells.shared)
    clog(trace) << "rank " << rank  << "shared cells " << itr << std::endl;
  for(auto itr: cells.ghost)
    clog(trace) << "rank " << rank  << "ghost cells " << itr << std::endl;

  // Create a map version for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    shared_cells_map;
  {
  for(auto i: cells.shared) {
    shared_cells_map[i.id] = i;
  } // for
  } // scope

  //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->gather_coloring_info(cell_color_info);

  // Add colorings to the context.
  context_.add_coloring(0, cells, cell_coloring_info);

  // Maps for output
  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    exclusive_cells_map;
  for(auto i: cells.exclusive) {
    exclusive_cells_map[i.id] = i;
  } // for

  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    ghost_cells_map;
  for(auto i: cells.ghost) {
    ghost_cells_map[i.id] = i;
  } // for

  // Gather primary partitions
  auto primary_cells = communicator->get_entity_reduction(cells.primary);


} // add_colorings

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//

static void create_gid_to_index_maps (
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        std::map<size_t, size_t>* map_gid_to_exclusive_index,
        std::map<size_t, size_t>* map_gid_to_shared_index,
        std::map<size_t, size_t>* map_gid_to_ghost_index);

static void calc_x_indices(const size_t gid_pt,
          size_t* gid_plus_i, size_t* gid_minus_i);

static void calc_y_indices(const size_t gid_pt,
          size_t* gid_plus_j, size_t* gid_minus_j);

static double initial_value(const size_t pt);


void calculate_exclusive_x_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update)
{
    const double adv = X_ADVECTION;

    std::map<size_t, size_t> map_gid_to_excl_index;
    std::map<size_t, size_t> map_gid_to_shared_index;
    std::map<size_t, size_t> map_gid_to_ghost_index;

    create_gid_to_index_maps(global_IDs, &map_gid_to_excl_index,
        &map_gid_to_shared_index, &map_gid_to_ghost_index);

    for (size_t index = 0; index < phi.exclusive_size(); index++) {
        size_t gid_plus_i, gid_minus_i;
        calc_x_indices(global_IDs(index), &gid_plus_i, &gid_minus_i);

        double update = -adv * adv * phi(index);

        if (map_gid_to_excl_index.find(gid_plus_i)
            != map_gid_to_excl_index.end())
                update += 0.5 * (adv * adv - adv)
                * phi(map_gid_to_excl_index.find(gid_plus_i)->second);
        else if (map_gid_to_shared_index.find(gid_plus_i)
            != map_gid_to_shared_index.end())
            update += 0.5 * (adv * adv - adv)
            * phi.shared(map_gid_to_shared_index.find(gid_plus_i)->second);
        else
          assert(false);

        if (map_gid_to_excl_index.find(gid_minus_i)
            != map_gid_to_excl_index.end())
            update += 0.5 * (adv * adv + adv)
            * phi(map_gid_to_excl_index.find(gid_minus_i)->second);
        else if (map_gid_to_shared_index.find(gid_minus_i)
            != map_gid_to_shared_index.end())
            update += 0.5 * (adv * adv + adv)
            * phi.shared(map_gid_to_shared_index.find(gid_minus_i)->second);
        else
          assert(false);

        phi_update(index) = update;
    }


}

void advect_owned_cells_in_x(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update)
{
  const double adv = X_ADVECTION;

  std::map<size_t, size_t> map_gid_to_excl_index;
  std::map<size_t, size_t> map_gid_to_shared_index;
  std::map<size_t, size_t> map_gid_to_ghost_index;

  create_gid_to_index_maps(global_IDs, &map_gid_to_excl_index,
      &map_gid_to_shared_index, &map_gid_to_ghost_index);

  for (size_t index = 0; index < phi.shared_size(); index++) {
      size_t gid_plus_i, gid_minus_i;
      calc_x_indices(global_IDs.shared(index), &gid_plus_i, &gid_minus_i);

      double update = -adv * adv * phi.shared(index);

      if (map_gid_to_shared_index.find(gid_plus_i)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi.shared(map_gid_to_shared_index.find(gid_plus_i)->second);
      else if (map_gid_to_ghost_index.find(gid_plus_i)
          != map_gid_to_ghost_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi.ghost(map_gid_to_ghost_index.find(gid_plus_i)->second);
      else if (map_gid_to_excl_index.find(gid_plus_i)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi(map_gid_to_excl_index.find(gid_plus_i)->second);
      else
        assert(false);

      if (map_gid_to_shared_index.find(gid_minus_i)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.shared(map_gid_to_shared_index.find(gid_minus_i)->second);
      else if (map_gid_to_ghost_index.find(gid_minus_i)
          != map_gid_to_ghost_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.ghost(map_gid_to_ghost_index.find(gid_minus_i)->second);
      else if (map_gid_to_excl_index.find(gid_minus_i)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi(map_gid_to_excl_index.find(gid_minus_i)->second);
      else
        assert(false);

      phi_update.shared(index) = update;
  }

  for (size_t index = 0; index < phi.exclusive_size(); index++)
      phi(index) += phi_update(index);

  for (size_t index = 0; index < phi.shared_size(); index++)
      phi.shared(index) += phi_update.shared(index);

}

void calculate_exclusive_y_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update)
{
  const double adv = Y_ADVECTION;

  std::map<size_t, size_t> map_gid_to_excl_index;
  std::map<size_t, size_t> map_gid_to_shared_index;
  std::map<size_t, size_t> map_gid_to_ghost_index;

  create_gid_to_index_maps(global_IDs, &map_gid_to_excl_index,
      &map_gid_to_shared_index, &map_gid_to_ghost_index);

  for (size_t index = 0; index < phi.exclusive_size(); index++) {
      size_t gid_plus_j, gid_minus_j;
      calc_y_indices(global_IDs(index), &gid_plus_j, &gid_minus_j);

      double update = -adv * adv * phi(index);

      if (map_gid_to_excl_index.find(gid_plus_j)
          != map_gid_to_excl_index.end())
              update += 0.5 * (adv * adv - adv)
              * phi(map_gid_to_excl_index.find(gid_plus_j)->second);
      else if (map_gid_to_shared_index.find(gid_plus_j)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi.shared(map_gid_to_shared_index.find(gid_plus_j)->second);
      else
        assert(false);

      if (map_gid_to_excl_index.find(gid_minus_j)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi(map_gid_to_excl_index.find(gid_minus_j)->second);
      else if (map_gid_to_shared_index.find(gid_minus_j)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.shared(map_gid_to_shared_index.find(gid_minus_j)->second);
      else
        assert(false);

      phi_update(index) = update;
  }

}

void advect_owned_cells_in_y(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update)
{
  const double adv = Y_ADVECTION;

  std::map<size_t, size_t> map_gid_to_excl_index;
  std::map<size_t, size_t> map_gid_to_shared_index;
  std::map<size_t, size_t> map_gid_to_ghost_index;

  create_gid_to_index_maps(global_IDs, &map_gid_to_excl_index,
      &map_gid_to_shared_index, &map_gid_to_ghost_index);

  for (size_t index = 0; index < phi.shared_size(); index++) {
      size_t gid_plus_j, gid_minus_j;
      calc_y_indices(global_IDs.shared(index), &gid_plus_j, &gid_minus_j);

      double update = -adv * adv * phi.shared(index);

      if (map_gid_to_shared_index.find(gid_plus_j)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi.shared(map_gid_to_shared_index.find(gid_plus_j)->second);
      else if (map_gid_to_ghost_index.find(gid_plus_j)
          != map_gid_to_ghost_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi.ghost(map_gid_to_ghost_index.find(gid_plus_j)->second);
      else if (map_gid_to_excl_index.find(gid_plus_j)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv - adv)
          * phi(map_gid_to_excl_index.find(gid_plus_j)->second);
      else
        assert(false);

      if (map_gid_to_shared_index.find(gid_minus_j)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.shared(map_gid_to_shared_index.find(gid_minus_j)->second);
      else if (map_gid_to_ghost_index.find(gid_minus_j)
          != map_gid_to_ghost_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.ghost(map_gid_to_ghost_index.find(gid_minus_j)->second);
      else if (map_gid_to_excl_index.find(gid_minus_j)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi(map_gid_to_excl_index.find(gid_minus_j)->second);
      else
        assert(false);

      phi_update.shared(index) = update;
  }

  for (size_t index = 0; index < phi.exclusive_size(); index++)
      phi(index) += phi_update(index);

  for (size_t index = 0; index < phi.shared_size(); index++)
      phi.shared(index) += phi_update.shared(index);

}

void write_to_disk(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        const int my_color)
{
    char buf[40];
    sprintf(buf,"lax%d.part", my_color);
    std::ofstream myfile;
    myfile.open(buf);

    for (size_t i = 0; i < phi.exclusive_size(); i++) {
        const size_t pt = global_IDs.exclusive(i);
        const size_t y_index = pt / NX;
        const size_t x_index = pt % NX;
        myfile << x_index << " " << y_index << " " << phi.exclusive(i) <<
                std::endl;
    }

    for (size_t i = 0; i < phi.shared_size(); i++) {
        const size_t pt = global_IDs.shared(i);
        const size_t y_index = pt / NX;
        const size_t x_index = pt % NX;
        myfile << x_index << " " << y_index << " " << phi.shared(i) <<
            std::endl;
    }

      myfile.close();
}

void initialize_data(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> phi)
{
  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
    exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
      flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    global_IDs(index) = exclusive.id;
    phi(index) = initial_value(global_IDs(index));
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
      flecsi::coloring::entity_info_t shared = *shared_itr;
    global_IDs(index) = shared.id;
    phi(index) = initial_value(global_IDs(index));
    index++;
  } // shared_itr
}

static void create_gid_to_index_maps (
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        std::map<size_t, size_t>* map_gid_to_exclusive_index,
        std::map<size_t, size_t>* map_gid_to_shared_index,
        std::map<size_t, size_t>* map_gid_to_ghost_index)
{
  // TODO profile effects of this indirection

  for (size_t index = 0; index < global_IDs.exclusive_size(); index++)
      (*map_gid_to_exclusive_index)[global_IDs(index)] = index;

  for (size_t index = 0; index < global_IDs.shared_size(); index++)
      (*map_gid_to_shared_index)[global_IDs.shared(index)] = index;

  for (size_t index = 0; index < global_IDs.ghost_size(); index++)
      (*map_gid_to_ghost_index)[global_IDs.ghost(index)] = index;


}

static void calc_x_indices(const size_t gid_pt,
          size_t* gid_plus_i, size_t* gid_minus_i)
{
  const size_t gid_y_index = gid_pt / NX;
  const size_t gid_x_index = gid_pt % NX;
  *gid_plus_i = (gid_x_index + 1) != NX ? gid_x_index + 1 + gid_y_index * NX
      : gid_y_index * NX;
  *gid_minus_i = gid_x_index != 0 ? gid_x_index - 1 + gid_y_index * NX:
      NX - 1 + gid_y_index * NX;
}

static void calc_y_indices(const size_t gid_pt,
          size_t* gid_plus_j, size_t* gid_minus_j)
{
  const size_t gid_y_index = gid_pt / NX;
  const size_t gid_x_index = gid_pt % NX;
  *gid_plus_j = (gid_y_index + 1) != NY ? gid_x_index + (1 + gid_y_index) * NX
      : gid_x_index;
  *gid_minus_j = gid_y_index != 0 ? gid_x_index + (gid_y_index - 1) * NX:
      gid_x_index + (NY - 1) * NX;
}

static double initial_value(const size_t pt) {
    double value = 0.0;
    const size_t y_index = pt / NX;
    const size_t x_index = pt % NX;
    double x = static_cast<double>(x_index) / static_cast<double>(NX - 1);
    double y = static_cast<double>(y_index) / static_cast<double>(NY - 1);
    if ( (x <= 0.5) && (y <= 0.5) )
        value = 1.0;
    return value;
}

int main(int argc, char ** argv) {


#ifdef GASNET_CONDUIT_MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // If you fail this assertion, then your version of MPI
    // does not support calls from multiple threads and you
    // cannot use the GASNet MPI conduit
    if (provided < MPI_THREAD_MULTIPLE)
      printf("ERROR: Your implementation of MPI does not support "
           "MPI_THREAD_MULTIPLE which is required for use of the "
           "GASNet MPI conduit with the Legion-MPI Interop!\n");
    assert(provided == MPI_THREAD_MULTIPLE);
#else
std::cout <<"MPI MPI  MPI"<< std::endl;
   MPI_Init(&argc, &argv);
#endif

  std::cout <<"lax-wendroff structured grid example"<<std::endl;

  // Call FleCSI runtime initialization
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif

  return retval;
} // main



/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
