/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef blender_field_info_h
#define blender_field_info_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

namespace blender {

//----------------------------------------------------------------------------//
//! Storage type for registered field information.
//----------------------------------------------------------------------------//

struct field_info_t
{
  std::string name;
  std::string nspace;
  std::string index_space;
}; // struct field_info_t

} // namespace blender

#endif // blender_field_info_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
