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
#pragma once

#include <flecsi-config.h>

#include <flecsi/utils/graphviz.h>
#include <flecsi-tutorial/specialization/control/control.h>
#include <unistd.h>

using namespace flecsi::tutorial;

/*
  Proxy function.
 */

int poynting_flux(int argc, char ** argv) {
  usleep(200000);
  std::cout << "analyze: poynting_flux" << std::endl;
  return 0;
} // poynting_flux

/*
  Register poynting_flux proxy function.
 */

register_action(analyze /* phase */,
  poynting_flux /* name */,
  poynting_flux /* action */);

/*
  Proxy function.
 */

int output_final(int argc, char ** argv) {
  usleep(200000);
  std::cout << "finalize: output_final" << std::endl;

  /*
    This section outputs the control graph for the application as a
    graphviz file.
   */

#if defined(FLECSI_ENABLE_GRAPHVIZ)
  auto & control = control_t::instance();
  flecsi::utils::graphviz_t gv;
  control.write(gv);
  gv.write("control.gv");
#endif

  return 0;
} // finalize

/*
  Register output_final proxy function.
 */

register_action(finalize /* phase */,
  output_final /* name */,
  output_final /* action */);
