/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#define DH2

#include <cinchtest.h>

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/empty_mesh_2d.h>

using namespace flecsi;
using namespace supplemental;

clog_register_tag(coloring);

using global_t = double;

//---------------------------------------------------------------------------//
// FleCSI tasks
//---------------------------------------------------------------------------//

void
task1(dense_accessor<double, ro, ro, ro> x, double y) {
  // np(y);
} // task1

void
data_handle_dump(dense_accessor<double, rw, ro, ro> x) {
  clog(info) << "combined size: " << x.size() << std::endl;
  clog(info) << "exclusive size: " << x.exclusive_size() << std::endl;
  clog(info) << "shared size: " << x.shared_size() << std::endl;
  clog(info) << "ghost size: " << x.ghost_size() << std::endl;
}

void
global_data_handle_dump(global_accessor<double, ro> x) {
  clog(info) << "global combined size: " << x.size() << std::endl;
}

void
color_data_handle_dump(color_accessor<double, ro> x) {
  clog(info) << "color combined size: " << x.size() << std::endl;
}

void
exclusive_writer(dense_accessor<double, wo, ro, ro> x) {
  clog(info) << "exclusive writer write" << std::endl;
  for (int i = 0; i < x.exclusive_size(); i++) {
    x(i) = static_cast<double>(i);
  }
}

void
exclusive_reader(dense_accessor<double, ro, ro, ro> x) {
  clog(info) << "exclusive reader read: " << std::endl;
  for (int i = 0; i < x.exclusive_size(); i++) {
    ASSERT_EQ(x(i), static_cast<double>(i));
  }
}

void
global_writer(global_accessor<double, wo> x) {
  clog(info) << "global writer write" << std::endl;
  x = static_cast<double>(3.14);
}

void
global_reader(global_accessor<double, ro> x) {
  clog(info) << "global reader read: " << std::endl;
  ASSERT_EQ(x, static_cast<double>(3.14));
}

void
color_writer(color_accessor<double, wo> x) {
  clog(info) << "color exclusive writer write" << std::endl;
  x = static_cast<double>(16);
}

void
color_reader(color_accessor<double, ro> x) {
  clog(info) << "color exclusive reader read: " << std::endl;
  ASSERT_EQ(x, static_cast<double>(16));
}

void
mpi_task(int val, global_accessor<double, ro> x) {
  std::cout << "This is an mpi task, val = " << val << std::endl;
  ASSERT_EQ(x, static_cast<double>(3.14));
}

void
exclusive_mpi(dense_accessor<double, ro, ro, ro> x) {
  clog(info) << "exclusive reader read: " << std::endl;
  for (int i = 0; i < x.exclusive_size(); i++) {
    ASSERT_EQ(x(i), static_cast<double>(i));
  }
}

//---------------------------------------------------------------------------//
// Tasks registration
//---------------------------------------------------------------------------//
flecsi_register_task_simple(task1, loc, single);
flecsi_register_task_simple(data_handle_dump, loc, single);
flecsi_register_task_simple(global_data_handle_dump, loc, single);
flecsi_register_task_simple(color_data_handle_dump, loc, single);
flecsi_register_task_simple(exclusive_writer, loc, single);
flecsi_register_task_simple(exclusive_reader, loc, single);
flecsi_register_task_simple(global_writer, loc, single);
flecsi_register_task_simple(global_reader, loc, single);
flecsi_register_task_simple(color_writer, loc, single);
flecsi_register_task_simple(color_reader, loc, single);
flecsi_register_task(mpi_task, , mpi, index);
flecsi_register_task(exclusive_mpi, , mpi, index);

//---------------------------------------------------------------------------//
// Data client registration
//---------------------------------------------------------------------------//
flecsi_register_data_client(empty_mesh_2d_t, meshes, mesh1);

//---------------------------------------------------------------------------//
// Fields
//---------------------------------------------------------------------------//
flecsi_register_field(empty_mesh_2d_t, ns, pressure, double, dense, 1, 0);

flecsi_register_global(ns, velocity, double, 1);

flecsi_register_global(ns, time, double, 1);

flecsi_register_color(ns, density, double, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  auto & context = execution::context_t::instance();

  ASSERT_EQ(
      context.execution_state(), static_cast<size_t>(SPECIALIZATION_TLT_INIT));

  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;

  flecsi_execute_mpi_task(add_colorings, flecsi::supplemental, map);

  auto global_handle = flecsi_get_global(ns, velocity, double, 0);
  flecsi_execute_task_simple(global_data_handle_dump, single, global_handle);
  flecsi_execute_task_simple(global_writer, single, global_handle);
  flecsi_execute_task_simple(global_reader, single, global_handle);
  flecsi_execute_task(mpi_task, , index, 10, global_handle);
  auto global_handle2 = flecsi_get_global(ns, time, double, 0);
  flecsi_execute_task_simple(global_writer, single, global_handle2);
  flecsi_execute_task(mpi_task, , index, 11, global_handle2);
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  auto & context = execution::context_t::instance();
  ASSERT_EQ(context.execution_state(), static_cast<size_t>(DRIVER));

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto ch = flecsi_get_client_handle(empty_mesh_2d_t, meshes, mesh1);

  auto h = flecsi_get_handle(ch, ns, pressure, double, dense, 0);

  //  flecsi_execute_task_simple(task1, single, h, 128);
  flecsi_execute_task_simple(data_handle_dump, single, h);
  flecsi_execute_task_simple(exclusive_writer, single, h);
  flecsi_execute_task_simple(exclusive_reader, single, h);

  // get global handle
  auto global_handle = flecsi_get_global(ns, velocity, double, 0);

  flecsi_execute_task_simple(global_data_handle_dump, single, global_handle);

  // get color handle
  auto color_handle = flecsi_get_color(ns, density, double, 0);

  flecsi_execute_task_simple(color_data_handle_dump, single, color_handle);
  flecsi_execute_task_simple(color_writer, single, color_handle);
  flecsi_execute_task_simple(color_reader, single, color_handle);
  flecsi_execute_task(mpi_task, , index, 10, global_handle);

  flecsi_execute_task(exclusive_mpi, , index, h);

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH2
