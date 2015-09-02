/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_mesh_entity_h
#define jali_mesh_entity_h

/*!
 * \file mesh_entity.h
 * \authors bergen
 * \date Initial file creation: Aug 18, 2015
 */

namespace jali {

/*!
  \class mesh_entity mesh_entity.h
  \brief mesh_entity provides a generic base class for mesh entities.
 */

struct mesh_entity_t {

  //! Destructor
  virtual ~mesh_entity_t() {}

}; // struct mesh_entity_t

} // namespace jali

#endif // jali_mesh_entity_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
