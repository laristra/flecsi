/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/coloring/dcrs_utils.h"
#include "flecsi/data/data.h"
#include "flecsi/geometry/point.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/supplemental/mesh/empty_mesh_2d.h"

using namespace flecsi;
using namespace supplemental;

clog_register_tag(coloring);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  data::legion::dense_handle_t<T, EP, SP, GP>;
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  data::mpi::dense_handle_t<T, EP, SP, GP>;
#endif

using point_t = flecsi::point__<double, 2>;

void exclusive_writer(handle_t<point_t, wo, ro, ro> x) {
  clog(info) << "exclusive writer write" << std::endl;
}

flecsi_register_task(exclusive_writer, loc, single);

namespace flecsi {
namespace coloring {

}
}
namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char **argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  coloring_map_t map;
  map.vertices = 1;
  map.cells = 0;

  flecsi_execute_mpi_task(add_colorings, map);
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char **argv) {
  clog(info) << "In driver" << std::endl;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto ch = flecsi_get_client_handle(empty_mesh_2d_t, meshes, mesh1);

  auto h = flecsi_get_handle(ch, ns, position, point_t , dense, 0);

  //flecsi_execute_task(data_handle_dump, single, h);
  flecsi_execute_task(exclusive_writer, single, h);
  //flecsi_execute_task(exclusive_reader, single, h);
} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(data_handle, testname) {

} // TEST

}
}