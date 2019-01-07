#pragma once

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#if defined(RISTRA_UTILS_ENABLE_GRAPHVIZ)
#include <flecsi/utils/graphviz.h>
#endif

#include <io-poc/control/control.h>
#include <unistd.h>

using namespace io_poc;

int restart_dump(int argc, char ** argv) {
  usleep(200000);
  std::cout << "io: restart_dump" << std::endl;
  return 0;
} // restart_dump

register_action(io /* phase */,
  restart_dump /* name */,
  restart_dump /* action */);

int output_final(int argc, char ** argv) {

#if defined(RISTRA_UTILS_ENABLE_GRAPHVIZ)
  usleep(200000);
  std::cout << "finalize: output_final" << std::endl;

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank == 0) {
    auto & control = control_t::instance();
    flecsi::utils::graphviz_t gv;
    control.write(gv);
    gv.write("control.gv");
  } // if
#endif

  return 0;
} // finalize

register_action(finalize, output_final, output_final);
