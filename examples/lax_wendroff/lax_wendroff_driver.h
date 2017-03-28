/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef lax_wendroff_driver_h
#define lax_wendroff_driver_h

#include <iostream>

#include "flecsi/execution/execution.h"
#include "flecsi/partition/weaver.h"

///
// \file lax_wendroff.h
// \authors jgraham, irina, bergen
// \date Initial file creation: Feb 2, 2017
///

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

#define NX 32
#define NY 32
#define CFL 0.5
#define U 1.0
#define V 1.0

namespace flecsi {
namespace execution {


template<typename T>
using accessor_t = flecsi::data::legion::dense_accessor_t<T, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;


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

static double get_x_velocity() {
  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  return U * dt / dx;
}

static double get_y_velocity() {
  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  return V * dt / dy;
}

void
initialize_data(
        accessor_t<size_t> global_IDs,
        accessor_t<double> acc_cells
)
{

    for (size_t i = 0; i < acc_cells.size(); i++)
        acc_cells[i] = initial_value(global_IDs[i]);

    for (size_t i = 0; i < acc_cells.shared_size(); i++)
        acc_cells.shared(i) = initial_value(global_IDs.shared(i));

}

flecsi_register_task(initialize_data, loc, single);

static void calc_x_indices(const size_t gid_pt,
          size_t* gid_plus_i, size_t* gid_minus_i)
{
  const size_t gid_y_index = gid_pt / NX;
  const size_t gid_x_index = gid_pt % NX;
  *gid_plus_i = (gid_x_index + 1) != NX ? gid_x_index + 1 + gid_y_index * NX: -1;
  *gid_minus_i = gid_x_index != 0 ? gid_x_index - 1 + gid_y_index * NX: -1;
}

static void calc_y_indices(const size_t gid_pt,
          size_t* gid_plus_j, size_t* gid_minus_j)
{
  const size_t gid_y_index = gid_pt / NX;
  const size_t gid_x_index = gid_pt % NX;
  *gid_plus_j = (gid_y_index + 1) != NY ? gid_x_index + (1 + gid_y_index) * NX: -1;
  *gid_minus_j = gid_y_index != 0 ? gid_x_index + (gid_y_index - 1) * NX: -1;
}

static void create_maps (accessor_t<size_t>& global_IDs,
        std::map<size_t, size_t>* exclusive_map,
        std::map<size_t, size_t>* shared_map,
        std::map<size_t, size_t>* ghost_map)
{
    // TODO profile effects of this indirection

    for (size_t i = 0; i < global_IDs.size(); i++)
        (*exclusive_map)[global_IDs[i]] = i;

    for (size_t i = 0; i < global_IDs.shared_size(); i++)
        (*shared_map)[global_IDs.shared(i)] = i;

    for (size_t i = 0; i < global_IDs.ghost_size(); i++)
        (*ghost_map)[global_IDs.ghost(i)] = i;

}

void
calculate_exclusive_x (
        accessor_t<size_t> global_IDs,
        accessor_t<double> phi,
        accessor_t<double> phi_update
)
{
    const double a = get_x_velocity();

    std::map<size_t, size_t> excl_map;
    std::map<size_t, size_t> shared_map;
    std::map<size_t, size_t> ghost_map;

    create_maps(global_IDs, &excl_map, &shared_map, &ghost_map);

    for (size_t index = 0; index < phi.size(); index++) {
        size_t gid_plus_i, gid_minus_i;
        calc_x_indices(global_IDs[index], &gid_plus_i, &gid_minus_i);

        double value = -a * a * phi[index];

        if (excl_map.find(gid_plus_i) != excl_map.end())
                value += 0.5 * (a * a - a) * phi[excl_map.find(gid_plus_i)->second];
        else if (shared_map.find(gid_plus_i) != shared_map.end())
            value += 0.5 * (a * a - a) * phi.shared(shared_map.find(gid_plus_i)->second);

        if (excl_map.find(gid_minus_i) != excl_map.end())
            value += 0.5 * (a * a + a) * phi[excl_map.find(gid_minus_i)->second];
        else if (shared_map.find(gid_minus_i) != shared_map.end())
            value += 0.5 * (a * a + a) * phi.shared(shared_map.find(gid_minus_i)->second);

        phi_update[index] = value;
    }

}

flecsi_register_task(calculate_exclusive_x, loc, single);

void
calculate_exclusive_y (
        accessor_t<size_t> global_IDs,
        accessor_t<double> phi,
        accessor_t<double> phi_update
)
{
    const double b = get_y_velocity();

    std::map<size_t, size_t> excl_map;
    std::map<size_t, size_t> shared_map;
    std::map<size_t, size_t> ghost_map;

    create_maps(global_IDs, &excl_map, &shared_map, &ghost_map);

    for (size_t index = 0; index < phi.size(); index++) {
        size_t gid_plus_j, gid_minus_j;
        calc_y_indices(global_IDs[index], &gid_plus_j, &gid_minus_j);

        double value = -b * b * phi[index];

        if (excl_map.find(gid_plus_j) != excl_map.end())
                value += 0.5 * (b * b - b) * phi[excl_map.find(gid_plus_j)->second];
        else if (shared_map.find(gid_plus_j) != shared_map.end())
            value += 0.5 * (b * b - b) * phi.shared(shared_map.find(gid_plus_j)->second);

        if (excl_map.find(gid_minus_j) != excl_map.end())
            value += 0.5 * (b * b + b) * phi[excl_map.find(gid_minus_j)->second];
        else if (shared_map.find(gid_minus_j) != shared_map.end())
            value += 0.5 * (b * b + b) * phi.shared(shared_map.find(gid_minus_j)->second);

        phi_update[index] = value;
    }
}

flecsi_register_task(calculate_exclusive_y, loc, single);

void
advect_own_x (
        accessor_t<size_t> global_IDs,
        accessor_t<double> phi,
        accessor_t<double> phi_update
)
{
    const double a = get_x_velocity();

    std::map<size_t, size_t> excl_map;
    std::map<size_t, size_t> shared_map;
    std::map<size_t, size_t> ghost_map;

    create_maps(global_IDs, &excl_map, &shared_map, &ghost_map);

    for (size_t index = 0; index < phi.shared_size(); index++) {
        size_t gid_plus_i, gid_minus_i;
        calc_x_indices(global_IDs.shared(index), &gid_plus_i, &gid_minus_i);

        double value = -a * a * phi.shared(index);

        if (shared_map.find(gid_plus_i) != shared_map.end())
            value += 0.5 * (a * a - a) * phi.shared(shared_map.find(gid_plus_i)->second);
        else if (ghost_map.find(gid_plus_i) != ghost_map.end())
            value += 0.5 * (a * a - a) * phi.ghost(ghost_map.find(gid_plus_i)->second);
        else if (excl_map.find(gid_plus_i) != excl_map.end())
            value += 0.5 * (a * a - a) * phi[excl_map.find(gid_plus_i)->second];

        if (shared_map.find(gid_minus_i) != shared_map.end())
            value += 0.5 * (a * a + a) * phi.shared(shared_map.find(gid_minus_i)->second);
        else if (ghost_map.find(gid_minus_i) != ghost_map.end())
            value += 0.5 * (a * a + a) * phi.ghost(ghost_map.find(gid_minus_i)->second);
        else if (excl_map.find(gid_minus_i) != excl_map.end())
            value += 0.5 * (a * a + a) * phi[excl_map.find(gid_minus_i)->second];

        phi_update.shared(index) = value;
    }

    for (size_t index = 0; index < phi.size(); index++)
        phi[index] += phi_update[index];

    for (size_t index = 0; index < phi.shared_size(); index++)
        phi.shared(index) += phi_update.shared(index);

}

flecsi_register_task(advect_own_x, loc, single);

void
advect_own_y (
        accessor_t<size_t> global_IDs,
        accessor_t<double> phi,
        accessor_t<double> phi_update
)
{
    const double b = get_y_velocity();

    std::map<size_t, size_t> excl_map;
    std::map<size_t, size_t> shared_map;
    std::map<size_t, size_t> ghost_map;

    create_maps(global_IDs, &excl_map, &shared_map, &ghost_map);

    for (size_t index = 0; index < phi.shared_size(); index++) {
        size_t gid_plus_j, gid_minus_j;
        calc_y_indices(global_IDs.shared(index), &gid_plus_j, &gid_minus_j);

        double value = -b * b * phi.shared(index);

        if (shared_map.find(gid_plus_j) != shared_map.end())
            value += 0.5 * (b * b - b) * phi.shared(shared_map.find(gid_plus_j)->second);
        else if (ghost_map.find(gid_plus_j) != ghost_map.end())
            value += 0.5 * (b * b - b) * phi.ghost(ghost_map.find(gid_plus_j)->second);
        else if (excl_map.find(gid_plus_j) != excl_map.end())
            value += 0.5 * (b * b - b) * phi[excl_map.find(gid_plus_j)->second];

        if (shared_map.find(gid_minus_j) != shared_map.end())
            value += 0.5 * (b * b + b) * phi.shared(shared_map.find(gid_minus_j)->second);
        else if (ghost_map.find(gid_minus_j) != ghost_map.end())
            value += 0.5 * (b * b + b) * phi.ghost(ghost_map.find(gid_minus_j)->second);
        else if (excl_map.find(gid_minus_j) != excl_map.end())
            value += 0.5 * (b * b + b) * phi[excl_map.find(gid_minus_j)->second];

        phi_update.shared(index) = value;
    }

    for (size_t index = 0; index < phi.size(); index++)
        phi[index] += phi_update[index];

    for (size_t index = 0; index < phi.shared_size(); index++)
        phi.shared(index) += phi_update.shared(index);

}

flecsi_register_task(advect_own_y, loc, single);

void
write_to_disk (
        accessor_t<size_t> global_IDs,
        accessor_t<double> phi,
        size_t my_color
)
{
  char buf[40];
  sprintf(buf,"lax%d.part", my_color);
  std::ofstream myfile;
  myfile.open(buf);

  for (size_t i = 0; i < phi.size(); i++) {
      const size_t pt = global_IDs[i];
      const size_t y_index = pt / NX;
      const size_t x_index = pt % NX;
      myfile << x_index << " " << y_index << " " << phi[i] << std::endl;
  }

  for (size_t i = 0; i < phi.shared_size(); i++) {
      const size_t pt = global_IDs.shared(i);
      const size_t y_index = pt / NX;
      const size_t x_index = pt % NX;
      myfile << x_index << " " << y_index << " " << phi.shared(i) << std::endl;
  }

    myfile.close();
}

flecsi_register_task(write_to_disk , loc, single);


void
driver(
  int argc,
  char ** argv
)
{
  flecsi::execution::context_t & context_ = flecsi::execution::context_t::instance();
  const LegionRuntime::HighLevel::Task *task = context_.task(flecsi::utils::const_string_t{"driver"}.hash());
  const size_t my_color = task->index_point.point_data[0];

  flecsi::data_client& dc = *((flecsi::data_client*)argv[argc - 1]);

  std::cout << my_color << " driver " << std::endl;


  size_t index_space = 0;

  auto write_exclusive_shared =
    flecsi_get_handle(dc, lax, phi, double, dense, index_space, rw, rw, none);
  auto rw_excl_shrd_ro_ghost =
    flecsi_get_handle(dc, lax, phi, double, dense, index_space, rw, rw, ro);
  auto cell_IDs =
   flecsi_get_handle(dc, lax, cell_ID, size_t, dense, index_space, ro, ro, ro);
  auto read_exclusive_shared =
   flecsi_get_handle(dc, lax, phi, double, dense, index_space, ro, ro, none);
  auto write_exclusive_update =
   flecsi_get_handle(dc, lax, phi_update, double, dense, index_space, rw, none, none);
  auto write_shared_update =
   flecsi_get_handle(dc, lax, phi_update, double, dense, index_space, ro, rw, none);

  flecsi_execute_task(initialize_data, loc, single, cell_IDs, write_exclusive_shared);

  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  double time = 0.0;
  while (time < 0.165) {
    time += dt;
    std::cout << "t=" << time << std::endl;

    flecsi_execute_task(calculate_exclusive_x, loc, single, cell_IDs, read_exclusive_shared,
            write_exclusive_update);

    flecsi_execute_task(advect_own_x, loc, single, cell_IDs, rw_excl_shrd_ro_ghost,
            write_shared_update);

    flecsi_execute_task(calculate_exclusive_y, loc, single, cell_IDs, read_exclusive_shared,
            write_exclusive_update);

    flecsi_execute_task(advect_own_y, loc, single, cell_IDs, rw_excl_shrd_ro_ghost,
            write_shared_update);

  }

  std::cout << "time " << time << std::endl;

  flecsi_execute_task(write_to_disk, loc, single, cell_IDs, read_exclusive_shared, my_color);

  std::cout << "lax wendroff ... all tasks issued"
  << std::endl;

} //driver

} // namespace execution
} // namespace flecsi

#endif // lax_wendroff_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=4 shiftwidth=4 expandtab :
 *~-------------------------------------------------------------------------~-*/
