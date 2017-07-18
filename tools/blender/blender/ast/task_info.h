/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef blender_task_info_h
#define blender_task_info_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

namespace blender {

//----------------------------------------------------------------------------//
//! Storage type for registered task information.
//----------------------------------------------------------------------------//

struct task_info_t
{
  std::string name;
  std::string processor;
  std::string launch;
}; // struct task_info_t

} // namespace blender

#endif // blender_task_info_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
