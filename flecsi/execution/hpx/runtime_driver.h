/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_hpx_runtime_driver_h
#define flecsi_execution_hpx_runtime_driver_h

///
/// \file
/// \date Initial file creation: Aug 01, 2016
///


namespace flecsi {
namespace execution {

/*!
 This is the specialization driver function to be defined by the FleCSI
 specialization layer. This symbol will be undefined in the compiled
 library, and is intended as a place holder for the specializations's
 driver function that will resolve the missing symbol.

 @param argc The number of arguments in argv (passed from the command line).
 @param argv The list of arguments (passed from the command line).

 @ingroup hpx-execution
 */

#if defined(FLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
void specialization_tlt_init(int argc, char ** argv);
#endif

#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
void specialization_spmd_init(int argc, char **argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

/*! @cond IGNORE */

void driver(int argc, char ** argv);

/*! @endcond */

/*!
 The FleCSI runtime driver task. This is just a function that is called
 during initialization. It will call the specialization driver and the
 driver functions as appropriate.

 @ingroup hpx-execution
 */
int hpx_runtime_driver(int argc, char ** argv);

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
