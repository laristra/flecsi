/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_vertex_h
#define jali_vertex_h

/*!
 * \file vertex.h
 * \authors bergen
 * \date Initial file creation: Aug 18, 2015
 */

#include "mesh_entity.h"

namespace jali {

/*!
  \class vertex vertex.h
  \brief vertex provides...
 */
template<typename vertex_policy_t>
class vertex_ : private vertex_policy_t, public mesh_entity_t
{
public:

  //! Default constructor
  vertex_(std::size_t id) : vertex_policy_t(id), mesh_entity_t() {}

  //! Copy constructor (disabled)
  vertex_(const vertex_ &) = delete;

  //! Assignment operator (disabled)
  vertex_ & operator = (const vertex_ &) = delete;

  //! Destructor
  virtual ~vertex_() {}

  std::size_t id() const { return vertex_policy_t::id(); }

}; // class vertex_

} // namespace jali

#endif // jali_vertex_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
