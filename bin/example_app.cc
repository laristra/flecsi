/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/execution/execution.h"

///
// \file example_app.cc
// \authors bergen
// \date Initial file creation: Aug 25, 2016
///

int main(int argc, char ** argv) {
  return flecsi::execution::context_t::instance().initialize(argc, argv);
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
