/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_default_mesh_policy_h
#define flexi_default_mesh_policy_h

/*!
 * \file default_policy.h
 * \authors bergen
 * \date Initial file creation: Aug 18, 2015
 */

#if defined(FLEXI_USER_MESH_POLICY)
  #include <flexi_mesh_policy.h>
#else

namespace flexi {

class default_policy_t
{
public:

  default_policy_t(std::size_t id) : id_(id) {}

protected:

  std::size_t id_;

}; // struct default_policy_t

  namespace mesh_policy {

    using vertex_t = default_policy_t;

  } // namespace mesh_policy

} // namespace flexi

#endif // if defined(FLEXI_USER_MESH_POLICY)

#endif // flexi_default_mesh_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
