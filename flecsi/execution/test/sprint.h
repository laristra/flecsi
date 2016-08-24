/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sprint_h
#define flecsi_sprint_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/partition/index_partition.h"

///
// \file sprint.h
// \authors bergen
// \date Initial file creation: Aug 23, 2016
///

namespace flecsi {
namespace execution {

using index_partition_t = dmp::index_partition__<size_t>;

static const size_t N = 8;

void mpi_task(double val) {
  int rank = 0;
  int size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  //std::cout << "My rank: " << rank << std::endl;

  size_t part = N/size;
  size_t rem = N%size;

  size_t start = rank*(part + (rem > 0 ? 1 : 0));
  size_t end = rank < rem ? start + part+1 : start + part;

  //std::cout << "rank: " << rank << " start: " << start <<
  //  " end: " << end << std::endl;

  index_partition_t ip;

  for(size_t j(0); j<N; ++j) {
    for(size_t i(start); i<end; ++i) {
      const size_t id = j*N+i;

      // independent
      if(i>start && i<end-1) {
        ip.independent.push_back(id);

        //std::cout << "rank: " << rank << " independent: " << id << std::endl;
      }
      else if(rank == 0 && i==start) {
        ip.independent.push_back(id);
        //std::cout << "rank: " << rank << " independent: " << id << std::endl;
      }
      else if(rank == size-1 && i==end-1) {
        ip.independent.push_back(id);
        //std::cout << "rank: " << rank << " independent: " << id << std::endl;
      }
      else if(i==start) {
        ip.shared.push_back(id);
        //std::cout << "rank: " << rank << " shared: " << id << std::endl;

        const size_t ghost_id = j*N+i-1;
        ip.shared.push_back(ghost_id);
        //std::cout << "rank: " << rank << " ghost: " << ghost_id << std::endl;
      }
      else if(i==end-1) {
        ip.shared.push_back(id);
        //std::cout << "rank: " << rank << " shared: " << id << std::endl;

        const size_t ghost_id = j*N+i+1;
        ip.shared.push_back(ghost_id);
        //std::cout << "rank: " << rank << " ghost: " << ghost_id << std::endl;
      } // if

    } // for
  } // for

} // mpi_task

register_task(mpi_task, mpi, void, double);

void driver(int argc, char ** argv) {
  execute_task(mpi_task, mpi, 1.0);
} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
