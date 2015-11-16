/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_entity_definition_h
#define flexi_entity_definition_h

/*!
 * \file entity_definition.h
 * \authors bergen
 * \date Initial file creation: Nov 12, 2015
 */

namespace flexi {

/*!
  \class entity_definition_t entity_definition.h
  \brief entity_definition_t provides...
 */
class entity_definition_t
{
public:

  //! Default constructor
  entity_definition_t() {}

  //! Copy constructor (disabled)
  entity_definition_t(const entity_definition_t &) = delete;

  //! Assignment operator (disabled)
  entity_definition_t & operator = (const entity_definition_t &) = delete;

  //! Destructor
  virtual ~entity_definition_t() {}

  /*!
    \brief Return the dimension of this entity type.
   */
  virtual size_t dimension() = 0;

  /*!
    \brief Return the number of sub-entities for this dimension
      and entity type.
   */
  virtual size_t sub_entities(size_t dimension) = 0;

  /*!
    \brief Return a sub-entity map for this dimension, sub-entity and position.
   */
  virtual size_t sub_entity_map(size_t dimension, size_t i, size_t p) = 0;

}; // class entity_definition_t

} // namespace flexi

#endif // flexi_entity_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
