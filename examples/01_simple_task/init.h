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

#ifndef init_h
#define init_h

#include "types.h"

int32_t init(burton_mesh_t & mesh) {

	mesh.init_parameters((height+1)*(width+1));

  std::vector<vertex_t*> vs;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = mesh.create_vertex({double(i), double(j)});
      vs.push_back(v);
    } // for
  } // for

  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      // go over vertices counter clockwise to define cell
      auto c = mesh.create_cell({vs[i + j * width1],
        vs[i + 1 + j * width1],
        vs[i + 1 + (j + 1) * width1],
        vs[i + (j + 1) * width1]});
    } // for
  } // for

  mesh.init();

	return 0;

} // init

#endif // init_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
