/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>
#include <mpi.h>

#include "flecsi/io/simple_definition.h"
#include "flecsi/partition/dcrs_utils.h"

clog_register_tag(dcrs);

DEVEL(dcrs) {
  clog_set_output_rank(0);

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  std::set<size_t> naive = flecsi::dmp::naive_partitioning(sd);

  {
  clog_tag_guard(dcrs);
  clog_container_one(info, "naive partitioning", naive, clog::space);
  } // guard

} // DEVEL

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
