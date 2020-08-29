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

#define __FLECSI_PRIVATE__
#include <flecsi/data.hh>

#include "flecsi/topo/canonical/interface.hh"

#include "flecsi/util/unit.hh"
#include <flecsi/data/accessor.hh>
#include <flecsi/exec/kernel_interface.hh>

#include <Kokkos_Core.hpp>

using namespace flecsi;

#if 0 
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
  clog(info) << "[" << rank << "] setting value" << std::endl;
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
  for(auto i : mesh.cells()) {
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
  color_accessor<int, rw> color) {

  flecsi::parallel_for(mesh.cells(),
    KOKKOS_LAMBDA(auto c) { pressure(c) = 1.0; }, std::string("init"));
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

#if defined(REALM_USE_CUDA)
flecsi_register_task_simple(init_toc, toc, index);
#else
flecsi_register_task_simple(init_toc, loc, index);
#endif

void
test(client_handle_t<test_mesh_t, ro> mesh,
  dense_accessor<double, ro, ro, ro> pressure,
  global_accessor_u<int, ro> global,
  color_accessor<int, ro> color) {
  //  sparse_accessor<double, rw, rw, rw> alpha) {

  std::cout << "Starting..." << std::endl;
  flecsi::parallel_for(mesh.cells(),
    KOKKOS_LAMBDA(auto c) {
      assert(pressure(c) == 1.0);
      assert(global == 2042);
      assert(color == 2);
    },
    std::string("test"));

  forall(c, mesh.cells(), "test2") {
    assert(pressure(c) == 1.0);
    assert(global == 2042);
    assert(color == 2);
  }; // forall

  // Test reduction sum
  int total_cells = mesh.cells().size();
  int total_sum = (total_cells - 1) * (total_cells) / 2;
  int res = 0;
  flecsi::parallel_reduce(mesh.cells(),
    KOKKOS_LAMBDA(auto c, int & up) { up += c; },
    flecsi::reducer::sum<int>(res), std::string("test"));
  assert(total_sum == res);
  int res2 = 0;
  reduceall(c, up, mesh.cells(), flecsi::reducer::sum<int>(res2), "test2") {
    up += c;
  };
  assert(total_sum == res2);

  // Test reduction prod
  double total_prod = pow(1.2, total_cells);
  double resd = 1;
  flecsi::parallel_reduce(mesh.cells(),
    KOKKOS_LAMBDA(auto c, double & up) { up *= 1.2; },
    flecsi::reducer::prod<double>(resd), std::string("test"));
  assert(fabs(total_prod - resd) < 1.0e-9);

  double tmp = 0;
  double dres = 0;
  // Test reduceall
  flecsi::reduceall(
    c, tmp, mesh.cells(), flecsi::reducer::sum<double>(dres), "test") {
    tmp += pressure(c);
  };
  assert(total_cells == dres);
  std::cout << "Finished!" << std::endl;
}

#if defined(REALM_USE_CUDA)
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
#endif

struct canon : topo::specialization<topo::canonical, canon> {
  enum index_space { vertices, cells };
  using index_spaces = has<cells, vertices>;
  using connectivities = util::types<from<cells, has<vertices>>>;

  static coloring color(std::string const &) {
    return {{40, 60}, 2};
  } // color
};

canon::slot canonical;
canon::cslot coloring;

const field<double>::definition<canon, canon::cells> cell_field;
auto pressure = cell_field(canonical);

const int mine = 35;
const std::size_t favorite = 3;
const double p0 = 3.5;

int
init(canon::accessor<wo> t, field<double>::accessor<wo> c) {
  UNIT {
    t.mine(0) = mine;
    t.get_connect<canon::cells, canon::vertices>()(0) = favorite;
    c(0) = p0;
  };
} // init

int
kokkos(canon::accessor<wo> t, field<double>::accessor<wo> c) {
  UNIT {};
}

int
kokkos_driver() {
  UNIT {

    flog(info) << "Inside user driver" << std::endl;
    Kokkos::print_configuration(std::cerr);

    // use canonical
    const std::string filename = "input.txt";
    coloring.allocate(filename);
    canonical.allocate(coloring.get());
    EXPECT_EQ(test<init>(canonical, pressure), 0);

    EXPECT_EQ(test<kokkos>(canonical, pressure), 0);

    // auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
    // auto ph = flecsi_get_handle(mh, hydro, pressure, double, dense, 0);
    // auto gh = flecsi_get_global(global, int1, int, 0);
    // auto ch = flecsi_get_color(color, int2, int, 0);
    // auto am = flecsi_get_mutator(mh, hydro, alpha, double, sparse, 0, 5);
    // auto ah = flecsi_get_handle(mh, hydro, alpha, double, sparse, 0);

    // flecsi_execute_task_simple(init, index, mh, ph, ch, am);
    // flecsi_execute_task_simple(test, index, mh, ph, gh, ch, ah);
    // flecsi_execute_task_simple(init_loc, index, mh, ph, ch, am);
    // flecsi_execute_task_simple(init_toc, index, mh, ph, ch);
    // flecsi_execute_task_simple(test, index, mh, ph, gh, ch);
  };
} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

flecsi::unit::driver<kokkos_driver> driver;
