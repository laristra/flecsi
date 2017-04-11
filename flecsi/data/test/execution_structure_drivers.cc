/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Apr 11, 2017
///

#include <cinchlog.h>

namespace flecsi {
namespace execution {

void specialization_driver(int argc, char ** argv) {
  clog(info) << "Executing specialization driver" << std::endl;
} // specialization_driver

void driver(int argc, char ** argv) {
  clog(info) << "Executing driver" << std::endl;
} // specialization_driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
