/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef legion_runtime_driver_h
#define legion_runtime_driver_h

#include <legion.h>

/*!
 * \file legion_runtime_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

namespace flecsi {

void legion_runtime_driver(const LegionRuntime::HighLevel::Task *task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime);

} // namespace flecsi

#endif // legion_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
