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
/// \date Initial file creation: Jul 3, 2018
///

#include <cmath>
#include <stdexcept>

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>

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
using field = sparse_accessor<double, EP, SP, GP>;

using sparse_mutator = sparse_mutator<double>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, meshes, mesh1);

flecsi_register_field(mesh_t, func, f, double, sparse, 1, index_spaces::cells);
flecsi_register_field(mesh_t, func, g, double, sparse, 1, index_spaces::cells);

//----------------------------------------------------------------------------//
// Init field
//----------------------------------------------------------------------------//

void
init(mesh<ro> mesh, sparse_mutator f, size_t field_idx) {
  for (auto c : mesh.cells(owned)) {
    auto idx = c->index();
    // domain is 0..2*pi in both x and y
    double x = (double)idx[1] / (double)(N - 1) * 2.0 * pi;
    double y = (double)idx[0] / (double)(N - 1) * 2.0 * pi;

    f(c, field_idx) = sin(x) * y + 0.5 * cos(2.0 * y);
  }
} // init

flecsi_register_task(init, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Check results
//----------------------------------------------------------------------------//

void
check_results(
    mesh<ro> mesh,
    field<ro, ro, ro> values,
    size_t field_idx,
    size_t global_target) {
  auto target = flecsi_get_global_object(global_target, global, vec_2d_t);
  auto rank = context_t::instance().color();

  for (auto c : mesh.cells(owned)) {
    auto v = values(c, field_idx);
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
    size_t f_idx,
    field<wo, wo, ro> deriv,
    size_t deriv_idx,
    bool div_x) {
  double h = 2.0 * pi / (double)(N - 1);

  for (auto c : mesh.cells(owned)) {
    if (div_x) {
      // central difference in x, backward or forward at boundaries
      if (c->left() == nullptr)
        deriv(c, deriv_idx) = (f(c->right(), f_idx) - f(c, f_idx)) / h;
      else if (c->right() == nullptr)
        deriv(c, deriv_idx) = (f(c, f_idx) - f(c->left(), f_idx)) / h;
      else
        deriv(c, deriv_idx) =
            (f(c->right(), f_idx) - f(c->left(), f_idx)) / (2.0 * h);
    } else {
      // central difference in y, backward or forward at boundaries
      if (c->below() == nullptr)
        deriv(c, deriv_idx) = (f(c->above(), f_idx) - f(c, f_idx)) / h;
      else if (c->above() == nullptr)
        deriv(c, deriv_idx) = (f(c, f_idx) - f(c->below(), f_idx)) / h;
      else
        deriv(c, deriv_idx) =
            (f(c->above(), f_idx) - f(c->below(), f_idx)) / (2.0 * h);
    }
  }
} // modify

flecsi_register_task(compute_deriv, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Copy field
//----------------------------------------------------------------------------//

void
copy(
    mesh<ro> mesh,
    field<ro, ro, ro> from,
    size_t from_idx,
    sparse_mutator to,
    size_t to_idx) {
  for (auto c : mesh.cells(owned)) {
    to(c, to_idx) = from(c, from_idx);
  }
} // copy

flecsi_register_task(copy, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();

  context_t::sparse_index_space_info_t isi;
  isi.index_space = index_spaces::cells;
  isi.max_entries_per_index = 10;
  isi.exclusive_reserve = 8192;
  context_t::instance().set_sparse_index_space_info(isi);
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
  auto fh = flecsi_get_handle(mh, func, f, double, sparse, 0);
  auto gh = flecsi_get_handle(mh, func, g, double, sparse, 0);
  auto fm = flecsi_get_mutator(mh, func, f, double, sparse, 0, 2);
  auto gm = flecsi_get_mutator(mh, func, g, double, sparse, 0, 2);

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

  // f[7] will be f
  flecsi_execute_task(init, flecsi::execution, single, mh, fm, 7);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fh, 7, global_f_target);

  // g[1] will also be f
  flecsi_execute_task(init, flecsi::execution, single, mh, gm, 1);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, gh, 1, global_f_target);

  // overwrite g[1] with x derivative of f[7], so it will be fx
  flecsi_execute_task(
      compute_deriv, flecsi::execution, single, mh, fh, 7, gh, 1, true);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, gh, 1, global_fx_target);

  // initialize g[1111111] with f
  flecsi_execute_task(init, flecsi::execution, single, mh, gm, 1111111);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, gh, 1111111,
      global_f_target);

  // copy g[1] to f[7777777]
  flecsi_execute_task(copy, flecsi::execution, single, mh, gh, 1, fm, 7777777);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, fh, 7777777,
      global_fx_target);

  // overwrite g[1111111] with y derivative of f[7777777], so it will be fxy
  flecsi_execute_task(
      compute_deriv, flecsi::execution, single, mh, fh, 7777777, gh, 1111111,
      false);
  flecsi_execute_task(
      check_results, flecsi::execution, single, mh, gh, 1111111,
      global_fxy_target);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(finite_difference_sparse, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
