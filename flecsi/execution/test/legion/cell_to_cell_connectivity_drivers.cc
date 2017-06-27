/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: June 26, 2017
///

#include <cinchlog.h>
#include <cinchtest.h>
#include <math.h>

#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/supplemental/coloring/add_colorings.h"

// FIXME: Must match add_colorings.cc
#define M 16
#define N 16

#define INDEX_ID 0
#define VERSIONS 1

#define DIFFUSIVITY 0.25
#define DH (8.0 / M)
#define DT (DH * DH * 0.25 / DIFFUSIVITY)
#define LENGTH (8.0 + DH)
#define EPS 1.0e-16
#define L_INF_ERROR 210.0

clog_register_tag(cell_to_cell_connectivity);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = flecsi::data::legion::dense_handle_t<T, EP, SP, GP>;

void check_values_task(
    handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> cell_ID,
    handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> T,
    const double time);
flecsi_register_task(check_values_task, flecsi::loc, flecsi::single);

void init_values_task(
    handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T);
flecsi_register_task(init_values_task, flecsi::loc, flecsi::single);

void fwd_euler_heat_task(
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> T,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T_temp);
flecsi_register_task(fwd_euler_heat_task, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

flecsi_register_field(client_type, name_space, cell_ID, size_t, dense,
    INDEX_ID, VERSIONS);
flecsi_register_field(client_type, name_space, T, double, dense,
    INDEX_ID, VERSIONS);
flecsi_register_field(client_type, name_space, T_temp, double, dense,
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
  const int my_color = runtime->find_local_MPI_rank();
  clog(trace) << "Rank " << my_color << " in driver" << std::endl;

  client_type client;

  auto cell_IDs = flecsi_get_handle(client, name_space, cell_ID, size_t, dense,
      INDEX_ID);
  auto Ts = flecsi_get_handle(client, name_space, T, double, dense,
      INDEX_ID);
  auto temp_Ts = flecsi_get_handle(client, name_space, T_temp, double, dense,
      INDEX_ID);

  flecsi_execute_task(init_values_task, single, cell_IDs, Ts);

  double time = 0.0;
  while(time < 4.0) {
    flecsi_execute_task(fwd_euler_heat_task, single, Ts, temp_Ts);
    time += DT;
  }

  flecsi_execute_task(check_values_task, single, cell_IDs, Ts, time);
} // driver

} // namespace execution
} // namespace flecsi

double analytic_heat_solution(size_t cell_ID, double time);

double initial_temperature(size_t cell_ID);

void init_values_task(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T) {
  clog(trace) << "init_values_task" << std::endl;

  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
    exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    cell_ID(index) = exclusive.id;
    T(index) = initial_temperature(cell_ID(index));
    clog(trace) << cell_ID(index) << " init'd as " << T(index) << std::endl;
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    cell_ID(index) = shared.id;
    T(index) = initial_temperature(cell_ID(index));
    clog(trace) << cell_ID(index) << " init'd as " << T(index) << std::endl;
    index++;
  } // shared_itr
} // init_values_task

double x_coord(size_t cell_ID) {
  return (cell_ID % M + 1.0) * DH;
}

double y_coord(size_t cell_ID) {
  return (cell_ID / M + 1.0) * DH;
}

void check_values_task(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> cell_ID,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> T,
        const double time) {
  clog(trace) << "check_values_task, t = " << time << std::endl;

  for(size_t i=0; i<(cell_ID.exclusive_size()+cell_ID.shared_size()); i++) {
    double expected = analytic_heat_solution(cell_ID(i), time);
    double error = fabs(expected - T(i));
    if(error > (L_INF_ERROR/(M*M)))
        clog(error) << cell_ID(i) << " actual " << T(i) << " expected " <<
            expected << " error " << error << " > " << (L_INF_ERROR/(M*M)) <<
            std::endl;
    assert(error <= (L_INF_ERROR/(M*M)));
  }  // for i
} // check_values_task

void fwd_euler_heat_task(
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> T,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T_temp) {
  clog(trace) << "fwd_euler_heat_task" << std::endl;
  // T_temp[i,j] = -4.0 T[i,j]
  // T_temp[i,j] += T[i+/-1,j+/-j] : no diagonals
  // T_temp[i,j] *= DIFFUSIVITY * DT / (DH*DH)
  // T[i,j] += T_temp[i,j]
}

double approximation_order(double time) {
  double error = EPS / (M * M);
  double order = 1025.0;
  if (time > 0.0)
    order = sqrt(-log(error) * LENGTH * LENGTH / (2.0 * DIFFUSIVITY * time
        * M_PI * M_PI));
  return order;
}

double analytic_heat_solution(size_t cell_ID, double time) {
  double approx_order = approximation_order(time);
  double analytic = 0.0;
  double base_spatial_coef = 4.0 * LENGTH * LENGTH / (M_PI * M_PI);
  double const_t_factor = -DIFFUSIVITY * time * M_PI * M_PI / (LENGTH * LENGTH);
  for(double m = 1.0; m < approx_order; m++)
    for(double n = 1.0; n < approx_order; n++) {
      double coef = base_spatial_coef / (m * n);
      if(((int)m+(int)n)%2 == 1)
        coef *= -1.0;
      double spatial = coef * sin(m * M_PI * x_coord(cell_ID) / LENGTH)
        * sin(n * M_PI * y_coord(cell_ID) / LENGTH);
      analytic += spatial * exp(const_t_factor * (m*m + n*n));
    }
  return analytic;
}

double initial_temperature(size_t cell_ID) {
  return x_coord(cell_ID) * y_coord(cell_ID);
}

TEST(cell_to_cell_connectivity, testname) {

} // TEST


/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
