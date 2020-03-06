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

#include <flecsi/coloring/box_types.h>

namespace flecsi {
namespace coloring {

/*!
   The box_colorer_t type provides an interface for creating distributed-memory
   colorings from an input grid-size.
   @ingroup coloring
 */
template<size_t D>
struct box_colorer_t {
  virtual box_coloring_info_t<D> color(size_t grid_size[D],
    size_t nhalo,
    size_t nhalo_domain,
    size_t thru_dim,
    size_t ncolors[D]) = 0;
}; // struct box_colorer_t

} // namespace coloring
} // namespace flecsi

#endif // box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
