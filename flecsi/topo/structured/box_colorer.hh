/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef box_colorer_h
#define box_colorer_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include "flecsi/topo/structured/box_types.hh"

namespace flecsi {
namespace topology {
namespace structured_impl {

/*!
   The box_colorer_t type provides an interface for creating distributed-memory
   colorings from an input grid-size.
   @ingroup coloring
 */
template<size_t D>
struct box_colorer_t {
  virtual ~box_colorer_t() {}; 

  virtual box_coloring_t color(
      size_t grid_size[D],
      size_t nghost_layers,
      size_t ndomain_layers,
      size_t thru_dim,
      size_t ncolors[D]) = 0;
}; // struct box_colorer_t

} // namespace structured_impl
} // namespace topology
} // namespace flecsi

#endif // box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
