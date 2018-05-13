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

//! \brief The main driver function to be defined by the user.
//! \param[in] argc  The number of arguments in argv.
//! \param[in] argv  The list arguments passed to the driver.
void driver(int argc, char ** argv);

//! \brief The specialization driver function to be defined by the user.
//! \param[in] argc  The number of arguments in argv.
//! \param[in] argv  The list arguments passed to the driver.
#if defined(FLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
void specialization_tlt_init(int argc, char ** argv);
#endif

#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
void specialization_spmd_init(int argc, char **argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

//! \brief The top-level serial runtime driver.
//! \param[in] argc  The number of arguments in argv.
//! \param[in] argv  The list arguments passed to the driver.
int hpx_runtime_driver(int argc, char ** argv);

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
