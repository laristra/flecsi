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

/*----------------------------------------------------------------------------*
  Documentation for this example can be found in README.md.
 *----------------------------------------------------------------------------*/

#if !defined(ENABLE_GRAPHVIZ)
#error ENABLE_GRAPHVIZ not defined! This file depends on Graphviz!
#endif

#include <flecsi/execution/execution.h>

#include "analysis.h"
#include "currents.h"
#include "fields.h"
#include "io.h"
#include "mesh.h"
#include "particles.h"
#include "special.h"
#include "species.h"

flecsi_register_program(control-model);
