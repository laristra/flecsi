/*----------------------------------------------------------------------------*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *----------------------------------------------------------------------------*/

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/execution/reduction.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

clog_register_tag(reduction_interface);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using point_t = flecsi::supplemental::point_t;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<size_t PS>
using mesh = data_client_handle_u<mesh_t, PS>;

template<size_t EP, size_t SP, size_t GP>
using field = dense_accessor<double, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//
flecsi_register_data_client(mesh_t, meshes, m);

flecsi_register_field(mesh_t,
  data,
  double_values,
  double,
  dense,
  1,
  index_spaces::cells);

//----------------------------------------------------------------------------//
// Double
//----------------------------------------------------------------------------//

void
double_init(mesh<ro> m, field<rw, rw, na> v) {
  for(auto c : m.cells(owned)) {
    v(c) = 1.0;
  } // for
} // double_init

flecsi_register_task(double_init, flecsi::execution, loc, index);

double
double_task(mesh<ro> m, field<rw, rw, ro> v) {
  double sum{0.0};

  for(auto c : m.cells(owned)) {
    sum += v(c);
  } // for

  return sum;
} // double_task

flecsi_register_task(double_task, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  {
    clog_tag_guard(devel_handle);
    clog(info) << "specialization_tlt_init function" << std::endl;
  } // scope

  supplemental::do_test_mesh_2d_coloring();
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_spmd_init(int argc, char ** argv) {
  {
    clog_tag_guard(devel_handle);
    clog(info) << "specialization_spmd_init function" << std::endl;
  } // scope

  auto mh = flecsi_get_client_handle(mesh_t, meshes, m);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_ini

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog_tag_guard(reduction_interface);

  auto mh = flecsi_get_client_handle(mesh_t, meshes, m);
  auto vh = flecsi_get_handle(mh, data, double_values, double, dense, 0);

  flecsi_execute_task(double_init, flecsi::execution, index, mh, vh);

  {
    auto f = flecsi_execute_reduction_task(
      double_task, flecsi::execution, index, min, double, mh, vh);

    double min = f.get();
    clog_assert(min == 64, "incorrect min from reduction");
  } // scope

  {
    auto f = flecsi_execute_reduction_task(
      double_task, flecsi::execution, index, max, double, mh, vh);
    double max = f.get();
    std::cout << "max = " << max << std::endl;
    // clog_assert(max == 64, "incorrect max from reduction");
  } // scope

  {
    auto f = flecsi_execute_reduction_task(
      double_task, flecsi::execution, index, sum, double, mh, vh);
    double sum = f.get();
    // clog_assert (min ==64, "incorrect sum from reduction");
  } // scope

  {
    auto f = flecsi_execute_reduction_task(
      double_task, flecsi::execution, index, product, double, mh, vh);
    double product = f.get();
    // clog_assert (product ==64, "incorrect product from reduction");

  } // scope
} // driver

} // namespace execution
} // namespace flecsi

TEST(reduction_interface, testname) {}

/*----------------------------------------------------------------------------*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *----------------------------------------------------------------------------*/
