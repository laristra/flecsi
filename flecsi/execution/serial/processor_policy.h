/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_serial_processor_policy_h
#define flecsi_execution_serial_processor_policy_h

///
/// \file
/// \date Initial file creation: Apr 12, 2017
///


namespace flecsi {
namespace execution {

enum class serial_processor_type_t : size_t {
  loc,
  toc,
  mpi
};

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_serial_processor_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
