/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_default_mesh_policy_h
#define jali_default_mesh_policy_h

/*!
 * \file default_policy.h
 * \authors bergen
 * \date Initial file creation: Aug 18, 2015
 */

#if defined(JALI_USER_MESH_POLICY)
  #include <jali_mesh_policy.h>
#else

namespace jali {

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

} // namespace jali

#endif // if defined(JALI_USER_MESH_POLICY)

#endif // jali_default_mesh_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
