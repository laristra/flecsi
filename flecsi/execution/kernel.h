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

#ifdef FLECSI_ENABLE_KOKKOS

#include <Kokkos_Core.hpp>

namespace flecsi {

/*!
  This function is a wrapper for Kokkos::parallel_for that has been adapted to
  work with FleCSI's topology iterator types. In particular, this function
  invokes a map from the normal kernel index space to the FleCSI index space,
  which may require indirection.
 */

template<typename ITERATOR, typename LAMBDA>
void
parallel_for(ITERATOR iterator, LAMBDA lambda, std::string const & name = "") {

  struct functor_t {

    functor_t(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i) const {
      lambda_(iterator_[i]);
    } // operator()

  private:
    ITERATOR & iterator_;
    LAMBDA & lambda_;

  }; // struct functor_t

  Kokkos::parallel_for(name, iterator.size(), functor_t{iterator, lambda});

} // parallel_for

template<typename ITERATOR>
struct forall_t {

  forall_t(ITERATOR iterator, std::string const & name = "")
    : iterator_(iterator) {}

  template<typename LAMBDA>
  struct functor_u {

    functor_u(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i) const {
      lambda_(iterator_[i]);
    } // operator()

  private:
    ITERATOR iterator_;
    LAMBDA lambda_;

  }; // struct functor_u

  template<typename LAMBDA>
  void operator+(LAMBDA lambda) {
    Kokkos::parallel_for(
      "  ", iterator_.size(), functor_u<LAMBDA>{iterator_, lambda});
  } // operator+

private:
  ITERATOR iterator_;

}; // forall_t

#define forall(it, iterator, name)                                             \
  forall_t{iterator, name} + KOKKOS_LAMBDA(auto it)

} // namespace flecsi
#endif

namespace flecsi {

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

} // namespace flecsi
