/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef box_colorer_hh
#define box_colorer_hh

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include "flecsi/topo/structured/box_types.hh"

namespace flecsi {
namespace topo {
namespace structured_impl {

/*!
   The box_colorer type provides an interface for creating distributed-memory
   colorings from an input grid-size.
   @ingroup coloring
 */
template<size_t D>
struct box_colorer {
  virtual ~box_colorer() {}; 

  virtual box_coloring color(
      size_t grid_size[D],
      size_t nghost_layers,
      size_t ndomain_layers,
      size_t thru_dim,
      size_t ncolors[D]) = 0;
}; // struct box_colorer

} // namespace structured_impl
} // namespace topo
} // namespace flecsi

#endif // box_colorer_hh
