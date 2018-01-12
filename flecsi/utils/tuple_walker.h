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


#include <tuple>

namespace flecsi {
namespace utils {

//----------------------------------------------------------------------------//
//! Helper type for iterating through tuples.
//!
//! @tparam INDEX      The inverse of the current index, i.e., the current
//!                    index is the tuple size minus INDEX.
//! @tparam TUPLE_TYPE The C++ type of the tuple.
//! @tparam CRTP_TYPE  The Curiously Recurring Template Pattern (CRTP) type.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

template<std::size_t INDEX, typename TUPLE_TYPE, typename CRTP_TYPE>
struct tuple_walker_helper__ {

  static constexpr std::size_t CURRENT =
      std::tuple_size<TUPLE_TYPE>::value - INDEX;

  using HELPER_TYPE = tuple_walker_helper__<INDEX - 1, TUPLE_TYPE, CRTP_TYPE>;

  //--------------------------------------------------------------------------//
  //! Walk the given tuple, calling the CRTP type's handle method.
  //!
  //! @param p An instance of the CRTP type.
  //! @param t The tuple instance.
  //--------------------------------------------------------------------------//

  static std::size_t walk(CRTP_TYPE & p, TUPLE_TYPE & t) {
    p.handle(std::get<CURRENT>(t));
    return HELPER_TYPE::walk(p, t);
  } // walk

  //--------------------------------------------------------------------------//
  //! Walk the given tuple by type, calling the CRTP type's handle method.
  //!
  //! @param p An instance of the CRTP type.
  //--------------------------------------------------------------------------//

  static std::size_t walk_types(CRTP_TYPE & p) {
    using ELEMENT_TYPE = typename std::tuple_element<CURRENT, TUPLE_TYPE>::type;
    p.template handle_type<ELEMENT_TYPE>();
    return HELPER_TYPE::walk_types(p);
  } // walk_types

}; // tuple_walker_helper__

//----------------------------------------------------------------------------//
//! Terminator type for tuple_walker_helper__.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

template<typename TUPLE_TYPE, typename CRTP_TYPE>
struct tuple_walker_helper__<0, TUPLE_TYPE, CRTP_TYPE> {

  //--------------------------------------------------------------------------//
  //! Termination of tuple walk.
  //--------------------------------------------------------------------------//

  static std::size_t walk(const CRTP_TYPE &, const TUPLE_TYPE &) {
    return 0;
  } // walk

  //--------------------------------------------------------------------------//
  //! Termination of tuple walk by type.
  //--------------------------------------------------------------------------//

  static std::size_t walk_types(const CRTP_TYPE &) {
    return 0;
  } // walk_types

}; // struct tuple_walker_helper__<0, TUPLE_TYPE, CRTP_TYPE>

//----------------------------------------------------------------------------//
//! Type for iterating through tuples. This type uses CRTP to expose handler
//! methods for processing the tuple arguments as they are traversed.
//!
//! @tparam CRTP_TYPE  The Curiously Recurring Template Pattern (CRTP) type.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

template<typename CRTP_TYPE>
struct tuple_walker__ {

  //--------------------------------------------------------------------------//
  //! Walk the given tuple, applying the handler to each element.
  //!
  //! @tparam TUPLE_TYPE The tuple type.
  //!
  //! @param t An instance of the tuple.
  //!
  //! @ingroup utils
  //--------------------------------------------------------------------------//

  template<typename TUPLE_TYPE>
  void walk(TUPLE_TYPE & t) {
    using HELPER_TYPE = tuple_walker_helper__<
        std::tuple_size<TUPLE_TYPE>::value, TUPLE_TYPE, CRTP_TYPE>;

    HELPER_TYPE::walk(*static_cast<CRTP_TYPE *>(this), t);
  } // walk

  //--------------------------------------------------------------------------//
  //! Walk the given tuple by type, applying the handler to each element.
  //!
  //! @tparam TUPLE_TYPE The tuple type.
  //!
  //! @ingroup utils
  //--------------------------------------------------------------------------//

  template<typename TUPLE_TYPE>
  void walk_types() {
    using HELPER_TYPE = tuple_walker_helper__<
        std::tuple_size<TUPLE_TYPE>::value, TUPLE_TYPE, CRTP_TYPE>;

    HELPER_TYPE::walk_types(*static_cast<CRTP_TYPE *>(this));
  } // walk_type

}; // struct tuple_walker__

} // namespace utils
} // namespace flecsi
