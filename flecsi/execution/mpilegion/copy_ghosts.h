/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_copy_ghosts_h
#define flecsi_copy_ghosts_h

using namespace LegionRuntime::HighLevel;

#include <legion.h>
///
/// \file
/// \author jgraham
/// \date Initial file creation: Mar 29, 2017
///


namespace flecsi {
namespace execution {
namespace mpilegion {

void
size_t_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
double_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);


}  //namespace mpilegion
} // namespace execution
} // namespace flecsi

#endif // flecsi_copy_ghosts_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
