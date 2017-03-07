/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_somethingelse_h
#define flecsi_execution_test_somethingelse_h

using namespace LegionRuntime::HighLevel;

#include <legion.h>
///
/// \file
/// \date Initial file creation: Dec 19, 2016
///



void
driver(
  int argc,
  char ** argv
)
{
	std::cout << "check GHOST" << std::endl;
} //driver



#endif // flecsi_execution_test_somethingelse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
