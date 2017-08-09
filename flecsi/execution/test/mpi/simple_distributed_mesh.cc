/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchlog.h>
#include <cinchtest.h>
#include <math.h>

#include "flecsi/execution/common/launch.h"
#include "flecsi/data/data.h"
#include "flecsi/execution/execution.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/topology/closure_utils.h"

#include "simple_distributed_mesh.h"

namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char ** argv) {
  flecsi_execute_mpi_task(add_colorings, 0);
} // specialization_tlt_init

void driver(int argc, char ** argv) {
  std::cout << "hello" << std::endl;
}

}
}

TEST(cell_to_cell_connectivity, testname) {

} // TEST
