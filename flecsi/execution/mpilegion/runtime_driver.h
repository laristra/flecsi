/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpilegion_runtime_driver_h
#define flecsi_execution_mpilegion_runtime_driver_h

#include <legion.h>

///
/// \file
/// \date Initial file creation: Jul 26, 2016
///  mpilegion_runtime_driver- an implementation of the legion top level task
///  that manages swithching between mpi and legion 
///

namespace flecsi {
namespace execution {

void 
mpilegion_runtime_driver(
  const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime
);

void
spmd_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);



} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpilegion_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
