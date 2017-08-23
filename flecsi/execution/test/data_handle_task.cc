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

#include "flecsi/execution/execution.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/execution_state.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/data/data.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/supplemental/mesh/empty_mesh_2d.h"

using namespace flecsi;
using namespace supplemental;

clog_register_tag(coloring);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  data::legion::dense_handle_t<T, EP, SP, GP>;

using global_t = double;

template<typename T, size_t P>
using global_handle_t =
  data::legion::global_handle_t<T, P>;

template<typename T, size_t P>
using color_handle_t =
  data::legion::color_handle_t<T, P>;

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  data::mpi::dense_handle_t<T, EP, SP, GP>;

#endif

void task1(handle_t<double, ro, ro, ro> x, double y) {
  //np(y);
} // task1

void data_handle_dump(handle_t<double, rw, ro, ro> x) {
  clog(info) << "label: " << x.label() << std::endl;
  clog(info) << "combined size: " << x.size() << std::endl;
  clog(info) << "exclusive size: " << x.exclusive_size() << std::endl;
  clog(info) << "shared size: " << x.shared_size() << std::endl;
  clog(info) << "ghost size: " << x.ghost_size() << std::endl;
}

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
void global_data_handle_dump(global_handle_t<double, ro> x) {
  clog(info) << "global label: " << x.label() << std::endl;
  clog(info) << "global combined size: " << x.size() << std::endl;
}

void color_data_handle_dump(color_handle_t<double, ro> x) {
  clog(info) << "color label: " << x.label() << std::endl;
  clog(info) << "color combined size: " << x.size() << std::endl;
}

#endif

void exclusive_writer(handle_t<double, wo, ro, ro> x) {
  clog(info) << "exclusive writer write" << std::endl;
  for (int i = 0; i < x.exclusive_size(); i++) {
    x(i) = static_cast<double>(i);
  }
}

void exclusive_reader(handle_t<double, ro, ro, ro> x) {
  clog(info) << "exclusive reader read: " << std::endl;
  for (int i = 0; i < x.exclusive_size(); i++) {
    ASSERT_EQ(x(i), static_cast<double>(i));
  }
}

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
void global_writer(global_handle_t<double, wo> x) {
  clog(info) << "global writer write" << std::endl;
    x = static_cast<double>(3.14);
}

void global_reader(global_handle_t<double, ro> x) {
  clog(info) << "global reader read: " << std::endl;
    ASSERT_EQ(x, static_cast<double>(3.14));
}

void color_writer(color_handle_t<double, wo> x) {
  clog(info) << "color exclusive writer write" << std::endl;
    x = static_cast<double>(16);
}

void color_reader(color_handle_t<double, ro> x) {
  clog(info) << "color exclusive reader read: " << std::endl;
    ASSERT_EQ(x, static_cast<double>(16));
}
#endif


flecsi_register_task(task1, loc, single);
flecsi_register_task(data_handle_dump, loc, single);
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
flecsi_register_task(global_data_handle_dump, loc, single);
flecsi_register_task(color_data_handle_dump, loc, single);
#endif
flecsi_register_task(exclusive_writer, loc, single);
flecsi_register_task(exclusive_reader, loc, single);
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
flecsi_register_task(global_writer, loc, single);
flecsi_register_task(global_reader, loc, single);
flecsi_register_task(color_writer, loc, single);
flecsi_register_task(color_reader, loc, single);
#endif
flecsi_register_field(empty_mesh_2d_t, ns, pressure, double, dense, 1, 0);

flecsi_register_global( ns, velocity, double, 1);

flecsi_register_field(empty_mesh_2d_t, ns, density, double, color, 1);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  auto & context = execution::context_t::instance();
 
  ASSERT_EQ(context.execution_state(),
    static_cast<size_t>(SPECIALIZATION_TLT_INIT));

  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;

  flecsi_execute_mpi_task(add_colorings, map);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
  auto global_handle = flecsi_get_global(ns, velocity, double, 0);
  flecsi_execute_task(global_data_handle_dump, single, global_handle);
  flecsi_execute_task(global_writer, single, global_handle);
  flecsi_execute_task(global_reader, single, global_handle);
#endif
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "In driver" << std::endl;

  auto & context = execution::context_t::instance();
  ASSERT_EQ(context.execution_state(), static_cast<size_t>(DRIVER));

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto ch = flecsi_get_client_handle(empty_mesh_2d_t, meshes, mesh1);

  auto h = flecsi_get_handle(ch, ns, pressure, double, dense, 0);

//  flecsi_execute_task(task1, single, h, 128);
  flecsi_execute_task(data_handle_dump, single, h);
  flecsi_execute_task(exclusive_writer, single, h);
  flecsi_execute_task(exclusive_reader, single, h);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
  //get global handle
  auto global_handle=flecsi_get_global(ns, velocity, double, 0);

  flecsi_execute_task(global_data_handle_dump, single, global_handle);

 //get color handle
  auto color_handle=flecsi_get_handle(ch, ns, density, double, color, 0);

  flecsi_execute_task(color_data_handle_dump, single, color_handle);
  flecsi_execute_task(color_writer, single, color_handle);
  flecsi_execute_task(color_reader, single, color_handle);
#endif

} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {
  
} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

#undef DH2
