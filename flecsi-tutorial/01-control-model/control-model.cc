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

#include <iostream>

#include <flecsi-tutorial/specialization/control/control.h>

#include "analysis.h"
#include "currents.h"
#include "fields.h"
#include "io.h"
#include "mesh.h"
#include "particles.h"
#include "species.h"

using namespace flecsi::tutorial;

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {
  auto & control = control_t::instance();
  control.execute(argc, argv);

#if defined(ENABLE_GRAPHVIZ)
  flecsi::utils::graphviz_t gv;
  control.write(gv);
  gv.write("control-model.gv");
#endif
} // driver

} // namespace execution
} // namespace flecsi
