/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_serial_runtime_driver_h
#define flecsi_execution_serial_runtime_driver_h

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
void specialization_driver(int argc, char ** argv);

//! \brief The top-level serial runtime driver.
//! \param[in] argc  The number of arguments in argv.
//! \param[in] argv  The list arguments passed to the driver.
void serial_runtime_driver(int argc, char ** argv);

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_serial_runtime_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
