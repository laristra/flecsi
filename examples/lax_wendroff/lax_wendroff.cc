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
#include "flecsi/supplemental/coloring/add_colorings.h"

#define INDEX_ID 0
#define VERSIONS 1
#define NX 8
#define NY 8
#define CFL 0.5
#define U 1.0
#define V 1.0
#define DX (1.0  / static_cast<double>(NX - 1))
#define DY (1.0  / static_cast<double>(NY - 1))
#define DT std::min(CFL * DX / U, CFL * DY / V)
#define X_ADVECTION U * DT / DX
#define Y_ADVECTION V * DT / DX

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  flecsi::data::legion::dense_handle_t<T, EP, SP, GP,
  flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t>>;

void init_global_IDs(
    handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> global_IDs,
    int my_color);
flecsi_register_task(init_global_IDs, flecsi::loc, flecsi::single);

void initialize_data(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> phi);
flecsi_register_task(initialize_data, flecsi::loc, flecsi::single);

void write_to_disk(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        const int my_color);
flecsi_register_task(write_to_disk, flecsi::loc, flecsi::single);

void calculate_exclusive_x_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update);
flecsi_register_task(calculate_exclusive_x_update, flecsi::loc, flecsi::single);

void advect_owned_cells_in_x(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update);
flecsi_register_task(advect_owned_cells_in_x, flecsi::loc, flecsi::single);

void calculate_exclusive_y_update(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> phi,
        handle_t<double, flecsi::drw, flecsi::dno, flecsi::dno> phi_update);
flecsi_register_task(calculate_exclusive_y_update, flecsi::loc, flecsi::single);

void advect_owned_cells_in_y(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dro> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> phi,
        handle_t<double, flecsi::dro, flecsi::drw, flecsi::dno> phi_update);
flecsi_register_task(advect_owned_cells_in_y, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

flecsi_new_register_data(client_type, lax, cell_ID, size_t, dense,
        INDEX_ID, VERSIONS);
flecsi_new_register_data(client_type, lax, phi, double, dense,
        INDEX_ID, VERSIONS);
flecsi_new_register_data(client_type, lax, phi_update, double, dense,
        INDEX_ID, VERSIONS);

namespace flecsi {
namespace execution {

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

  client_type client;

  auto phi_handle =
          flecsi_get_handle(client, lax, phi, double, dense, INDEX_ID);
  auto phi_update_handle =
          flecsi_get_handle(client, lax, phi_update, double, dense, INDEX_ID);
  auto global_IDs_handle =
          flecsi_get_handle(client, lax, cell_ID, size_t, dense, INDEX_ID);

  flecsi_execute_task(init_global_IDs, single, global_IDs_handle, my_color);
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

        if (map_gid_to_excl_index.find(gid_minus_i)
            != map_gid_to_excl_index.end())
            update += 0.5 * (adv * adv + adv)
            * phi(map_gid_to_excl_index.find(gid_minus_i)->second);
        else if (map_gid_to_shared_index.find(gid_minus_i)
            != map_gid_to_shared_index.end())
            update += 0.5 * (adv * adv + adv)
            * phi.shared(map_gid_to_shared_index.find(gid_minus_i)->second);

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

      if (map_gid_to_excl_index.find(gid_minus_j)
          != map_gid_to_excl_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi(map_gid_to_excl_index.find(gid_minus_j)->second);
      else if (map_gid_to_shared_index.find(gid_minus_j)
          != map_gid_to_shared_index.end())
          update += 0.5 * (adv * adv + adv)
          * phi.shared(map_gid_to_shared_index.find(gid_minus_j)->second);

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

      phi_update.shared(index) = update;
  }

  for (size_t index = 0; index < phi.exclusive_size(); index++)
      phi(index) += phi_update(index);

  for (size_t index = 0; index < phi.shared_size(); index++)
      phi.shared(index) += phi_update.shared(index);

}

void init_global_IDs(
    handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> global_IDs,
    int my_color)
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
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
      flecsi::coloring::entity_info_t shared = *shared_itr;
    global_IDs(index) = shared.id;
    index++;
  } // shared_itr

} // init_global_IDs

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
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> global_IDs,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> phi)
{
    size_t index = 0;
    for (size_t i=0; i < global_IDs.exclusive_size(); i++) {
        phi(index) = initial_value(global_IDs.exclusive(i));
        index++;
    }

    for (size_t i=0; i < global_IDs.shared_size(); i++) {
        phi(index) = initial_value(global_IDs.shared(i));
        index++;
    }
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
      : -1;
  *gid_minus_i = gid_x_index != 0 ? gid_x_index - 1 + gid_y_index * NX: -1;
}

static void calc_y_indices(const size_t gid_pt,
          size_t* gid_plus_j, size_t* gid_minus_j)
{
  const size_t gid_y_index = gid_pt / NX;
  const size_t gid_x_index = gid_pt % NX;
  *gid_plus_j = (gid_y_index + 1) != NY ? gid_x_index + (1 + gid_y_index) * NX
      : -1;
  *gid_minus_j = gid_y_index != 0 ? gid_x_index + (gid_y_index - 1) * NX: -1;
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
