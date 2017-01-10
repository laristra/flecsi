/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_task_init_h
#define flecsi_execution_task_init_h

#include "flecsi/execution/context.h"

///
/// \file
/// \date Initial file creation: Dec 21, 2016
///

void
inline
test_init(
  int argc,
  char ** argv
)
{
  // Add task registration here...

  // This will initialize the legion runtime, so you need to
  // do anything else before you get here.
  flecsi::execution::context_t::instance().initialize(argc, argv);
} // test_init

#endif // flecsi_execution_task_init_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
