/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_runtime_driver_h
#define flecsi_execution_legion_runtime_driver_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <legion.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! This is the specialization driver function to be defined by the FleCSI
//! specialization layer. This symbol will be undefined in the compiled
//! library, and is intended as a place holder for the specializations's
//! driver function that will resolve the missing symbol.
//!
//! @param argc The number of arguments in argv (passed from the command line).
//! @param argv The list of arguments (passed from the command line).
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#if defined FLECSI_ENABLE_SPECIALIZATION_DRIVER
void specialization_driver(int argc, char ** argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_DRIVER

//----------------------------------------------------------------------------//
//! This is the main driver function to be defined by the user. This symbol
//! will be undefined in the compiled library, and is intended as a place
//! holder for the user's driver function that will resolve the missing
//! symbol.
//!
//! @param argc The number of arguments in argv (passed from the command line).
//! @param argv The list of arguments (passed from the command line).
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv);

//----------------------------------------------------------------------------//
//! The FleCSI runtime driver task. This is the top-level Legion task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

void runtime_driver(const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime);

// The runtime_driver task is registered directly during context
// initialization.

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
