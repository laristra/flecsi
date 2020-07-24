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

/*! @file */

#include <vector>
#include <tuple> 
#include <utility>

namespace flecsi {
namespace topology {
namespace structured_impl {

template< template<size_t, size_t> class T, size_t N, size_t I>
using T_ = T<N, I>;

template <template<size_t, size_t> class T, size_t N, typename I>
struct selector;

template <template<size_t, size_t> class T, size_t N, size_t... Is>
struct selector<T, N, std::index_sequence<Is...>>
{
    using type = std::tuple<T_<T, N, Is>...>;
};

template <template<size_t,size_t> class T, size_t N>
struct gen_tuple_type
{
    using indices = std::make_index_sequence<N+1>;
    using type = typename selector<T, N, indices>::type;
};



/*----------------------------------------------------------------------------*
 * Tuple search utilities.
 *----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------//
//! Search tuples with two entries, index-space and entity type
//----------------------------------------------------------------------------//

template<size_t I, class T, size_t DIM>
struct find_entity_st__ {

  //--------------------------------------------------------------------------//
  //!
  //!
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Find the position index of a type that matches the
  //! given dimension.
  //!
  //! @tparam I The current index in tuple.
  //! @tparam T The tuple type.
  //! @tparam DIM The dimension to match.
  //! @tparam DOM The domain to match.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using E = typename std::tuple_element<I - 1, T>::type;
    using T1 = typename std::tuple_element<1, E>::type;

    // Check match for domain and dimension and return
    // index if matched or recurse if not matched.
    return T1::dimension == DIM
               ? I
               : find_entity_st__<I - 1, T, DIM>::find();
  }

}; // find_entity__

//-----------------------------------------------------------------//
//! \struct find_entity__ mesh_utils.h
//! \brief find_entity__ provides a specialization for the root recursion.
//-----------------------------------------------------------------//
template<class T, size_t DIM>
struct find_entity_st__<0, T, DIM> {
  //-----------------------------------------------------------------//
  //! Search last tuple element.
  //!
  //! @tparam T The tuple type.
  //! @tparam DIM The dimension to match.
  //! @tparam DOM The domain to match.
  //-----------------------------------------------------------------//
  static constexpr size_t find() {
    return 1;
  } // find
}; // struct find_entity__

//-----------------------------------------------------------------//
//! \struct find_entity_ mesh_utils.h
//! \brief find_entity_ provides static search capabilities.
//!
//! Top-level interface for recursive type search matching dimension and
//! domain.
//-----------------------------------------------------------------//
template<class MESH_TYPE, size_t DIM>
struct find_entity_st_ {
  using entity_types = typename MESH_TYPE::entity_types;

  using pair_ = typename std::tuple_element<
      find_entity_st__<
          std::tuple_size<entity_types>::value,
          entity_types,
          DIM>::find() -
          1,
      entity_types>::type;

  //-----------------------------------------------------------------//
  //! Define the type returned by searching the tuple for matching
  //! dimension and domain.
  //-----------------------------------------------------------------//
  using type = typename std::tuple_element<1, pair_>::type;
};


} // namespace structured_impl
} // namespace topology
} // namespace flecsi
