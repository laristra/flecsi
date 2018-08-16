/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

///
/// \file
/// \date Initial file creation: Apr 3, 2018
///

#include <cmath>
#include <stdexcept>

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <flecsi/data/dense_accessor.h>

static constexpr double pi = 3.14159265358979323846;
#ifdef FLECSI_8_8_MESH
static constexpr size_t N = 8;
#else
static constexpr size_t N = 16;
#endif

enum global_object_identifier_t : size_t {
  global_f_target,
  global_fx_target,
  global_fy_target,
  global_fxy_target
};

namespace flecsi {
namespace execution {

typedef std::vector<std::vector<double>> vec_2d_t;
flecsi_register_global_object(global_f_target, global, vec_2d_t);
flecsi_register_global_object(global_fx_target, global, vec_2d_t);
flecsi_register_global_object(global_fy_target, global, vec_2d_t);
flecsi_register_global_object(global_fxy_target, global, vec_2d_t);

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<size_t PS>
using mesh = data_client_handle__<mesh_t, PS>;

template<size_t EP, size_t SP, size_t GP>
using field = dense_accessor<double, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, meshes, mesh1);

flecsi_register_field(mesh_t, func, f, double, dense, 1, index_spaces::cells);
flecsi_register_field(mesh_t, deriv, fx, double, dense, 1, index_spaces::cells);
flecsi_register_field(mesh_t, deriv, fy, double, dense, 1, index_spaces::cells);
flecsi_register_field(
    mesh_t,
    deriv,
    fxy,
    double,
    dense,
    1,
    index_spaces::cells);

//----------------------------------------------------------------------------//
// Init field
//----------------------------------------------------------------------------//

void
init(mesh<ro> mesh, field<rw, rw, ro> f) {
  for (auto c : mesh.cells(owned)) {
    auto idx = c->index();
    // domain is 0..2*pi in both x and y
    double x = (double)idx[1] / (double)(N - 1) * 2.0 * pi;
    double y = (double)idx[0] / (double)(N - 1) * 2.0 * pi;

    f(c) = sin(x) * y + 0.5 * cos(2.0 * y);
  }
} // init

flecsi_register_task(init, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Check results
//----------------------------------------------------------------------------//

void
check_results(mesh<ro> mesh, field<ro, ro, ro> values, size_t global_target) {
  auto target = flecsi_get_global_object(global_target, global, vec_2d_t);
  auto rank = context_t::instance().color();

  for (auto c : mesh.cells(owned)) {
    auto v = values(c);
    size_t i = c->index()[0];
    size_t j = c->index()[1];
    auto t = (*target)[i][j];

    if (v != t) {
      printf("[Rank %lu] at [%lu,%lu] %.15e != %.15e\n", rank, i, j, v, t);
      throw std::runtime_error("Got wrong result");
    }
  }
} // print

flecsi_register_task(check_results, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Compute derivative
//----------------------------------------------------------------------------//

void
compute_deriv(
    mesh<ro> mesh,
    field<ro, ro, ro> f,
    field<wo, wo, ro> deriv,
    bool div_x) {
  double h = 2.0 * pi / (double)(N - 1);

  for (auto c : mesh.cells(owned)) {
    if (div_x) {
      // central difference in x, backward or forward at boundaries
      if (c->left() == nullptr)
        deriv(c) = (f(c->right()) - f(c)) / h;
      else if (c->right() == nullptr)
        deriv(c) = (f(c) - f(c->left())) / h;
      else
        deriv(c) = (f(c->right()) - f(c->left())) / (2.0 * h);
    } else {
      // central difference in y, backward or forward at boundaries
      if (c->below() == nullptr)
        deriv(c) = (f(c->above()) - f(c)) / h;
      else if (c->above() == nullptr)
        deriv(c) = (f(c) - f(c->below())) / h;
      else
        deriv(c) = (f(c->above()) - f(c->below())) / (2.0 * h);
    }
  }
} // modify

flecsi_register_task(compute_deriv, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, single, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
derive(const vec_2d_t & f, vec_2d_t * deriv, bool div_x) {
  double h = 2.0 * pi / (double)(N - 1);

  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      if (div_x) {
        // central difference in x, backward or forward at boundaries
        if (j == 0)
          (*deriv)[i][j] = (f[i][j + 1] - f[i][j]) / h;
        else if (j == N - 1)
          (*deriv)[i][j] = (f[i][j] - f[i][j - 1]) / h;
        else
          (*deriv)[i][j] = (f[i][j + 1] - f[i][j - 1]) / (2.0 * h);
      } else {
        // central difference in y, backward or forward at boundaries
        if (i == 0)
          (*deriv)[i][j] = (f[i + 1][j] - f[i][j]) / h;
        else if (i == N - 1)
          (*deriv)[i][j] = (f[i][j] - f[i - 1][j]) / h;
        else
          (*deriv)[i][j] = (f[i + 1][j] - f[i - 1][j]) / (2.0 * h);
      }
    }
  }
}

void
driver(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  auto fh = flecsi_get_handle(mh, func, f, double, dense, 0);
  auto fxh = flecsi_get_handle(mh, deriv, fx, double, dense, 0);
  auto fyh = flecsi_get_handle(mh, deriv, fy, double, dense, 0);
  auto fxyh = flecsi_get_handle(mh, deriv, fxy, double, dense, 0);

  // compute expected results and then compare with what FleCSI produces
  vec_2d_t f_target(N, std::vector<double>(N, 0.0));
  vec_2d_t fx_target(N, std::vector<double>(N, 0.0));
  vec_2d_t fy_target(N, std::vector<double>(N, 0.0));
  vec_2d_t fxy_target(N, std::vector<double>(N, 0.0));

  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      double x = (double)j / (double)(N - 1) * 2.0 * pi;
      double y = (double)i / (double)(N - 1) * 2.0 * pi;
      f_target[i][j] = sin(x) * y + 0.5 * cos(2.0 * y);
    }
  }

  derive(f_target, &fx_target, true);
  derive(f_target, &fy_target, false);
  derive(fx_target, &fxy_target, false);

  flecsi_initialize_global_object(global_f_target, global, vec_2d_t, f_target);
  flecsi_initialize_global_object(
      global_fx_target, global, vec_2d_t, fx_target);
  flecsi_initialize_global_object(
      global_fy_target, global, vec_2d_t, fy_target);
  flecsi_initialize_global_object(
      global_fxy_target, global, vec_2d_t, fxy_target);

  flecsi_execute_task(init, flecsi::execution, single, mh, fh);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fh, global_f_target);

  flecsi_execute_task(
      compute_deriv, flecsi::execution, single, mh, fh, fxh, true);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fxh, global_fx_target);

  flecsi_execute_task(
      compute_deriv, flecsi::execution, single, mh, fh, fyh, false);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fyh, global_fy_target);

  flecsi_execute_task(
      compute_deriv, flecsi::execution, single, mh, fxh, fxyh, false);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fxyh, global_fxy_target);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(finite_difference_dense, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
