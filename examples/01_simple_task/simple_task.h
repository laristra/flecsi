/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef driver_h
#define driver_h

#include <iostream>

#include "types.h"
#include "init.h"
#include "update.h"

int32_t driver(int argc, char ** argv) {

  burton_mesh_t mesh;

  // execute a task to initialize the mesh
  execute(init, mesh);

  // register several state variables
  register_state(mesh, "pressure", cells, real_t, persistent);
  register_state(mesh, "density", cells, real_t);
  register_state(mesh, "result", cells, real_t);

  // get accessors to the date
  auto p = access_state(mesh, "pressure", real_t);
  auto d = access_state(mesh, "density", real_t);

  // initialize state variables
  for(auto c: mesh.cells()) {
    p[c] = 1.0;
    d[c] = 1.0;
  } // for

  // get accessor
  auto r = access_state(mesh, "result", real_t);

  // execute update
  execute(update, mesh, p, d, r);

  std::cout << "Result:" << std::endl;
  for(auto c: mesh.cells()) {
    std::cout << r[c] << std::endl;
  } // for

  return 0;

} // driver

#endif // driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
