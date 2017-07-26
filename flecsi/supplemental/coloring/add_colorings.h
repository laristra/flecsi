/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef add_colorings_h
#define add_colorings_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 23, 2017
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

struct coloring_map_t
{
  size_t vertices;
  size_t cells;
}; // struct coloring_map_t

void add_colorings(coloring_map_t map);

} // namespace execution
} // namespace flecsi

#endif // add_colorings_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
