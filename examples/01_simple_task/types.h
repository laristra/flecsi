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

#ifndef type_h
#define type_h

#include <flecsi/specializations/burton/burton.h>
#include <flecsi/execution/task.h>

using burton_mesh_t = flecsi::burton_mesh_t;
using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;
using real_t = burton_mesh_t::real_t;
using accessor_t = burton_mesh_t::accessor_t<real_t>;
using flecsi::persistent;

const size_t height(2);
const size_t width(5);

#endif // type_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
