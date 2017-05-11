/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#include <cinchtest.h>
#include <iostream>

#include "flecsi/data/data.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/utils/common.h"

///
/// \file
/// \date Initial file creation: Jul 24, 2016
///

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Function registration.
//----------------------------------------------------------------------------//

double test_function(double r, double e) {
  std::cout << "Executing test_function" << std::endl;
  std::cout << "(r,e): (" << r << "," << e << ")" << std::endl;
  return r*e;
} // function1

flecsi_register_function(test_function);

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto handle = flecsi_function_handle(test_function);

  double result = flecsi_execute_function(handle, 2.0, 10.0);

  ASSERT_EQ(result, 20);

} // driver

} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
