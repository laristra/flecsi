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

#include <flecsi/execution/execution.h>
#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_GRAPHVIZ)
#error ENABLE_GRAPHVIZ not defined! \
  This example depends on Graphviz!
#endif

#if !defined(FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL)
#error FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL not defined! \
  This example depends on the FleCSI Dynamic Control Model
#endif

#include "analysis.h"
#include "currents.h"
#include "fields.h"
#include "io.h"
#include "mesh.h"
#include "particles.h"
#include "special.h"
#include "species.h"

flecsi_register_program(control-model);
