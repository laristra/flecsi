/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_kernel_h
#define flecsi_execution_kernel_h


#include "flecsi/topology/index_space.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 06, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! Abstraction function for fine-grained, data-parallel interface.
//!
//! @tparam ENTITY_TYPE The entity type of the associated index space.
//! @tparam STORAGE     A boolean indicating whether or not the associated
//!                     index space has storage for the referenced entity types.
//! @tparam OWNED       A boolean indicating whether or not the entity data are
//!                     owned by the associated index space.
//! @tparam SORTED      A boolean indicating whether or not the associated index
//!                     space is sorted.
//! @tparam PREDICATE   An optional predicate function used to select
//!                     indices matching particular criteria.
//! @tparam FUNCTION    The calleable object type.
//!
//! @param index_space  The index space over which to execute the calleable
//!                     object.
//! @param function     The calleable object instance.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<
  typename ENTITY_TYPE,
  bool STORAGE,
  bool OWNED,
  bool SORTED,
  typename PREDICATE,
  typename FUNCTION
>
inline
void
for_each__(
  flecsi::topology::index_space<
    ENTITY_TYPE,
    STORAGE,
    OWNED,
    SORTED,
    PREDICATE
  > & index_space,
  FUNCTION && function
)
{
  const size_t end = index_space.end_offset();

  for(size_t i(index_space.begin_offset()); i<end; ++i) {
    function(std::forward<ENTITY_TYPE>(index_space.get_offset(i)));
  } // for
} // for_each__

//----------------------------------------------------------------------------//
//! Abstraction function for fine-grained, data-parallel interface.
//!
//! @tparam ENTITY_TYPE The entity type of the associated index space.
//! @tparam STORAGE     A boolean indicating whether or not the associated
//!                     index space has storage for the referenced entity types.
//! @tparam OWNED       A boolean indicating whether or not the entity data are
//!                     owned by the associated index space.
//! @tparam SORTED      A boolean indicating whether or not the associated index
//!                     space is sorted.
//! @tparam PREDICATE   An optional predicate function used to select
//!                     indices matching particular criteria.
//! @tparam FUNCTION    The calleable object type.
//! @tparam REDUCTION   The reduction variabel type.
//!
//! @param index_space  The index space over which to execute the calleable
//!                     object.
//! @param function     The calleable object instance.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<
  typename ENTITY_TYPE,
  bool STORAGE,
  bool OWNED,
  bool SORTED,
  typename PREDICATE,
  typename FUNCTION,
  typename REDUCTION
>
inline
void
reduce_each__(
  flecsi::topology::index_space<
    ENTITY_TYPE,
    STORAGE,
    OWNED,
    SORTED,
    PREDICATE
  > & index_space,
  REDUCTION & reduction,
  FUNCTION && function
)
{
  size_t end = index_space.end_offset();

  for(size_t i(index_space.begin_offset()); i<end; ++i) {
    function(std::forward<ENTITY_TYPE>(index_space.get_offset(i)), reduction);
  } // for
} // reduce_each__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_kernel_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
