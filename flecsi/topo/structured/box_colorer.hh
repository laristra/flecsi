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
template<std::size_t D>
struct box_colorer {
  virtual ~box_colorer(){};

  virtual box_coloring color(std::size_t grid_size[D],
    std::size_t nghost_layers,
    std::size_t ndomain_layers,
    std::size_t thru_dim,
    std::size_t ncolors[D]) = 0;
}; // struct box_colorer

} // namespace structured_impl
} // namespace topo
} // namespace flecsi

#endif // box_colorer_hh
