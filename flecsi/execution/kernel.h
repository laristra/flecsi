/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file
 */

#include <flecsi/topology/index_space.h>

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

template<typename ENTITY_TYPE,
  bool STORAGE,
  bool OWNED,
  bool SORTED,
  typename PREDICATE,
  typename FUNCTION>
inline void
for_each_u(
  flecsi::topology::
    index_space_u<ENTITY_TYPE, STORAGE, OWNED, SORTED, PREDICATE> & index_space,
  FUNCTION && function) {
  const size_t end = index_space.end_offset();

  for(size_t i(index_space.begin_offset()); i < end; ++i) {
    function(std::forward<ENTITY_TYPE>(index_space.get_offset(i)));
  } // for
} // for_each_u

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

template<typename ENTITY_TYPE,
  bool STORAGE,
  bool OWNED,
  bool SORTED,
  typename PREDICATE,
  typename FUNCTION,
  typename REDUCTION>
inline void
reduce_each_u(
  flecsi::topology::
    index_space_u<ENTITY_TYPE, STORAGE, OWNED, SORTED, PREDICATE> & index_space,
  REDUCTION & reduction,
  FUNCTION && function) {
  size_t end = index_space.end_offset();

  for(size_t i(index_space.begin_offset()); i < end; ++i) {
    function(std::forward<ENTITY_TYPE>(index_space.get_offset(i)), reduction);
  } // for
} // reduce_each_u

} // namespace execution
} // namespace flecsi
