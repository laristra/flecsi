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

namespace flecsi {
namespace topology {

/*---------------------------------------------------------------------------*
 * Tuple search utilities.
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------//
//!
//---------------------------------------------------------------------------//

template<size_t I, typename TUPLE_TYPE, size_t FIND_INDEX_SPACE>
struct find_set_entity__ {

  //-------------------------------------------------------------------------//
  //!
  //!
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  //! Find the position index of a type that matches the
  //! given index space.
  //!
  //! @tparam I The current index in tuple.
  //! @tparam INDEX_SPACE index space.
  //-------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using E = typename std::tuple_element<I - 1, TUPLE_TYPE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, E>::type;

    // Check match for index space or recurse if not matched.
    return INDEX_SPACE::value == FIND_INDEX_SPACE
               ? I
               : find_set_entity__<I - 1, TUPLE_TYPE, FIND_INDEX_SPACE>::find();
  }

}; // find_set_entity__

//-----------------------------------------------------------------//
//! \struct find_set_entity__ set_utils.h
//! \brief find_set_entity__ provides a specialization for the root recursion.
//-----------------------------------------------------------------//
template<typename TUPLE_TYPE, size_t FIND_INDEX_SPACE>
struct find_set_entity__<0, TUPLE_TYPE, FIND_INDEX_SPACE> {
  //-----------------------------------------------------------------//
  //! Search last tuple element.
  //!
  //! @tparam T The tuple type.
  //! @tparam D The dimension to match.
  //! @tparam M The domain to match.
  //-----------------------------------------------------------------//
  static constexpr size_t find() {
    assert(false && "failed to find set entity");
    return 0;
  } // find
}; // struct find_set_entity__

//-----------------------------------------------------------------//
//! \struct find_set_entity_ set_utils.h
//! \brief find_set_entity_ provides static search capabilities.
//!
//! Top-level interface for recursive type search matching dimension and
//! domain.
//-----------------------------------------------------------------//
template<class SET_TYPES, size_t FIND_INDEX_SPACE>
struct find_set_entity_ {
  using entity_types = typename SET_TYPES::entity_types;

  using element = typename std::tuple_element<
      find_set_entity__<
          std::tuple_size<entity_types>::value,
          entity_types,
          FIND_INDEX_SPACE>::find() -
          1,
      entity_types>::type;

  //-----------------------------------------------------------------//
  //! Define the type returned by searching the tuple for matching
  //! dimension and domain.
  //-----------------------------------------------------------------//
  using type = typename std::tuple_element<1, element>::type;
};

template<size_t INDEX, class TUPLE, class ENTITY>
struct find_set_index_space__ {
  static constexpr size_t find() {
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<1, TUPLE_ELEMENT>::type;

    return std::is_same<ELEMENT_ENTITY, ENTITY>::value
               ? INDEX_SPACE::value
               : find_set_index_space__<INDEX - 1, TUPLE, ENTITY>::find();
  }
};

template<class TUPLE, class ENTITY>
struct find_set_index_space__<0, TUPLE, ENTITY> {

  static constexpr size_t find() {
    assert(false && "failed to find index space");
    return 1;
  }
};

template<size_t INDEX, class TUPLE, class MAP_TYPE>
struct map_set_index_spaces__ {
  static constexpr size_t map(MAP_TYPE & m) {
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;

    m[INDEX_SPACE::value] = INDEX - 1;

    return map_set_index_spaces__<INDEX - 1, TUPLE, MAP_TYPE>::map(m);
  }
};

template<class TUPLE, class MAP_TYPE>
struct map_set_index_spaces__<0, TUPLE, MAP_TYPE> {
  static constexpr size_t map(MAP_TYPE & m) {
    return 0;
  }
};

} // namespace topology
} // namespace flecsi
