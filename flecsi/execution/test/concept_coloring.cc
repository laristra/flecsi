/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <cinchdevel.h>

#include <flecsi/supplemental/coloring/concept_coloring.h>

namespace flecsi {
namespace execution {

void specialization_tlt_init(int argc, char ** argv) {
  flecsi_execute_mpi_task(concept_coloring, flecsi::execution);
} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
} // specialization_spmd_init

void driver(int argc, char ** argv) {
} // driver

} // namespace execution
} // namespace flecsi

DEVEL(concept_coloring) {}
