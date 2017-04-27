/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_runtime_driver_h
#define flecsi_execution_mpi_runtime_driver_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

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
//! @ingroup mpi-execution
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
//! @ingroup mpi-execution
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv);

//----------------------------------------------------------------------------//
//! The FleCSI runtime driver task. This is just a function that is called
//! during initialization. It will call the specialization driver and the 
//! driver functions as appropriate.
//!
//! @ingroup mpi-execution
//----------------------------------------------------------------------------//

void runtime_driver(int argc, char ** argv);

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
