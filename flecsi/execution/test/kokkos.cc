/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include <cinchlog.h>
#include <cinchtest.h>

#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/global_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/execution/kernel.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <Kokkos_Core.hpp>

namespace flecsi {
namespace execution {

// ----------- data registration ----------------------------------------//
using test_mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<typename DC, size_t PS>
using client_handle_t = data_client_handle_u<DC, PS>;

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_field(test_mesh_t,
  hydro,
  pressure,
  double,
  dense,
  1,
  index_spaces::cells);

flecsi_register_field(test_mesh_t,
  hydro,
  alpha,
  double,
  sparse,
  1,
  index_spaces::cells);

flecsi_register_global(global, int1, int, 1);
flecsi_register_color(color, int2, int, 1);

// ----------- tasks ----------------------------------------------------//

void
set_global_int(global_accessor_u<int, rw> global, int value) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();
  std::cout << "[" << rank << "] setting value" << std::endl;
  global = value;
}

flecsi_register_task_simple(set_global_int, loc, single);

void
init_loc(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, rw, rw, na> pressure,
  color_accessor<int, rw> color,
  sparse_mutator<double> sm) {

  color = 2;

  for(auto c : mesh.cells()) {
    pressure(c) = 0.0;
  }

  auto rank = execution::context_t::instance().color();
  for (auto i:mesh.cells()){
    auto gid = i->gid();
    // for most cells, do a checkerboard pattern
    bool parity = (gid / 8 + gid % 8) & 1;
    int start = (parity ? 0 : 1);
    int stop = (parity ? 6 : 5);
    // make a few cells overflow
    if(gid >= 11 && gid <= 13)
      stop = (parity ? 16 : 17);
    for(size_t j = start; j < stop; j += 2) {
      sm(i, j) = rank * 10000 + gid * 100 + j;
    }
  }

}

flecsi_register_task_simple(init_loc, loc, index);

void
init_toc(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, rw, rw, na> pressure,
  color_accessor<int, rw> color){

  flecsi::parallel_for(
    mesh.cells(), KOKKOS_LAMBDA(auto c) { pressure(c) = 1.0; },
    std::string("init"));
#if 0
  auto rank = execution::context_t::instance().color();
  forall(mesh.cells(), "init2") {
    auto gid = i->gid();
    // for most cells, do a checkerboard pattern
    bool parity = (gid / 8 + gid % 8) & 1;
    int start = (parity ? 0 : 1);
    int stop = (parity ? 6 : 5);
    // make a few cells overflow
    if(gid >= 11 && gid <= 13)
      stop = (parity ? 16 : 17);
    for(size_t j = start; j < stop; j += 2) {
    //  sm(i, j) = rank * 10000 + gid * 100 + j;
    }
  };
#endif
}

#if defined (REALM_USE_CUDA)
flecsi_register_task_simple(init_toc, toc, index);
#else
flecsi_register_task_simple(init_toc, loc, index);
#endif


void
test(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, ro, ro, ro> pressure,
  global_accessor_u<int, ro> global,
  color_accessor<int, ro> color){
//  sparse_accessor<double, rw, rw, rw> alpha) {

  flecsi::parallel_for(
    mesh.cells(),
    KOKKOS_LAMBDA(auto c) {
      assert(pressure(c) == 1.0);
      assert(global == 2042);
      assert(color == 2);
      //      for(auto entry : alpha.entries(c)) {
      //        std::cout << c->id() << ":" << entry << ": " << alpha(c, entry)
      //                  << std::endl;
      //      }
    },
    std::string("test"));

  forall(c, mesh.cells(), "test2") {
    assert(pressure(c) == 1.0);
    assert(global == 2042);
    assert(color == 2);
  }; // forall
}

#if defined (REALM_USE_CUDA)
flecsi_register_task_simple(test, toc, index);
#else
flecsi_register_task_simple(test, loc, index);
#endif
//----------------------------------------------------------------------------//
// Specialization driver.
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

  auto gh = flecsi_get_global(global, int1, int, 0);
  flecsi_execute_task_simple(set_global_int, single, gh, 2042);

} // specialization_tlt_init

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {

  clog(info) << "Inside user driver" << std::endl;

  Kokkos::print_configuration(std::cerr);

  auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(mh, hydro, pressure, double, dense, 0);
  auto gh = flecsi_get_global(global, int1, int, 0);
  auto ch = flecsi_get_color(color, int2, int, 0);
  auto am = flecsi_get_mutator(mh, hydro, alpha, double, sparse, 0, 5);
  auto ah = flecsi_get_handle(mh, hydro, alpha, double, sparse, 0);

 // flecsi_execute_task_simple(init, index, mh, ph, ch, am);
 // flecsi_execute_task_simple(test, index, mh, ph, gh, ch, ah);
 flecsi_execute_task_simple(init_loc, index, mh, ph, ch, am);
 flecsi_execute_task_simple(init_toc, index, mh, ph, ch);
 flecsi_execute_task_simple(test, index, mh, ph, gh, ch);

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {} // TEST

} // namespace execution
} // namespace flecsi
