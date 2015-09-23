/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_entity_types_h
#define flexi_entity_types_h

/*!
 * \file entity_types.h
 * \authors bergen
 * \date Initial file creation: Aug 18, 2015
 */

#include "default_mesh_policy.h"
#include "vertex.h"

namespace flexi {

  using vertex_t = vertex_<mesh_policy::vertex_t>;

} // namespace flexi

#endif // flexi_entity_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
