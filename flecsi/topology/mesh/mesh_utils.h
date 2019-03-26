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

#include <flecsi/utils/common.h>
#include <flecsi/utils/static_verify.h>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * Tuple search utilities.
 *----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<size_t I, class T, size_t DIM, size_t DOM>
struct find_entity_u {

  //--------------------------------------------------------------------------//
  //!
  //!
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Find the position index of a type that matches the
  //! given domain and dimension.
  //!
  //! @tparam I The current index in tuple.
  //! @tparam T The tuple type.
  //! @tparam DIM The dimension to match.
  //! @tparam DOM The domain to match.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using E = typename std::tuple_element<I - 1, T>::type;
    using D1 = typename std::tuple_element<1, E>::type;
    using T1 = typename std::tuple_element<2, E>::type;

    // Check match for domain and dimension and return
    // index if matched or recurse if not matched.
    return D1::value == DOM && T1::dimension == DIM
             ? I
             : find_entity_u<I - 1, T, DIM, DOM>::find();
  }

}; // find_entity_u

//-----------------------------------------------------------------//
//! \struct find_entity_u mesh_utils.h
//! \brief find_entity_u provides a specialization for the root recursion.
//-----------------------------------------------------------------//
template<class T, size_t DIM, size_t DOM>
struct find_entity_u<0, T, DIM, DOM> {
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
}; // struct find_entity_u

//-----------------------------------------------------------------//
//! \struct find_entity_ mesh_utils.h
//! \brief find_entity_ provides static search capabilities.
//!
//! Top-level interface for recursive type search matching dimension and
//! domain.
//-----------------------------------------------------------------//
template<class MESH_TYPE, size_t DIM, size_t DOM>
struct find_entity_ {
  using entity_types = typename MESH_TYPE::entity_types;

  using pair_ = typename std::tuple_element<
    find_entity_u<std::tuple_size<entity_types>::value,
      entity_types,
      DIM,
      DOM>::find() -
      1,
    entity_types>::type;

  //-----------------------------------------------------------------//
  //! Define the type returned by searching the tuple for matching
  //! dimension and domain.
  //-----------------------------------------------------------------//
  using type = typename std::tuple_element<2, pair_>::type;
};

template<size_t INDEX, class TUPLE, class ENTITY>
struct find_index_space_u {

  //--------------------------------------------------------------------------//
  //!
  //!
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Find the index corresponding to an entity type in the connectivities
  //! tuple - either from or to
  //!
  //! @tparam I The current index in tuple.
  //! @tparam T The tuple type.
  //! @tparam E The entity type to find.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<2, TUPLE_ELEMENT>::type;

    // Check match for domain and dimension and return
    // index if matched or recurse if not matched.
    return std::is_same<ELEMENT_ENTITY, ENTITY>::value
             ? INDEX_SPACE::value
             : find_index_space_u<INDEX - 1, TUPLE, ENTITY>::find();
  }

}; // find_index_space_u

//-----------------------------------------------------------------//
//! \struct find_entity_u mesh_utils.h
//! \brief find_entity_u provides a specialization for the root recursion.
//-----------------------------------------------------------------//

template<class TUPLE, class ENTITY>
struct find_index_space_u<0, TUPLE, ENTITY> {

  //-----------------------------------------------------------------//
  //! Search last tuple element.
  //!
  //!  @tparam T The tuple type.
  //!  @tparam DIM The dimension to match.
  //!  @tparam DOM The domain to match.
  //-----------------------------------------------------------------//
  static constexpr size_t find() {
    assert(false && "failed to find index space");
    return 1;
  } // find_from

}; // struct find_index_space_u

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<size_t INDEX, typename TUPLE, size_t DIM, size_t DOM>
struct find_index_space_from_dimension_u {
  //--------------------------------------------------------------------------//
  //! Find the index corresponding to an entity type in the connectivities
  //! tuple - either from or to
  //!
  //! @tparam I The current index in tuple.
  //! @tparam T The tuple type.
  //! @tparam E The entity type to find.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ENTITY_DOMAIN = typename std::tuple_element<1, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<2, TUPLE_ELEMENT>::type;

    // Check match for dimension and return if matched, recurse otherwise.
    return (DIM == ELEMENT_ENTITY::dimension && DOM == ENTITY_DOMAIN::value)
             ? INDEX_SPACE::value
             : find_index_space_from_dimension_u<INDEX - 1, TUPLE, DIM,
                 DOM>::find();
  } // find

}; // find_index_space_from_dimension_u

//----------------------------------------------------------------------------//
//! End recursion condition.
//----------------------------------------------------------------------------//

template<typename TUPLE, size_t DIM, size_t DOM>
struct find_index_space_from_dimension_u<0, TUPLE, DIM, DOM> {

  //--------------------------------------------------------------------------//
  //! Search last tuple element.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    return 1;
  } // find

}; // struct find_index_space_from_dimension_u

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<size_t INDEX, typename TUPLE, size_t ID>
struct find_index_space_from_id_u {
  //--------------------------------------------------------------------------//
  //! Find the index corresponding to an entity type in the connectivities
  //! tuple - either from or to
  //!
  //! @tparam INDEX The current index in tuple.
  //! @tparam TUPLE The tuple type.
  //! @tparam ID Index space id to search for.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;

    // Check match for ID and return if matched, recurse otherwise.
    return ID == INDEX_SPACE::value
             ? INDEX - 1
             : find_index_space_from_id_u<INDEX - 1, TUPLE, ID>::find();
  } // find

}; // find_index_space_from_id_u

//----------------------------------------------------------------------------//
//! End recursion condition.
//----------------------------------------------------------------------------//

template<typename TUPLE, size_t ID>
struct find_index_space_from_id_u<0, TUPLE, ID> {

  //--------------------------------------------------------------------------//
  //! Search last tuple element.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    assert("didnt find index space");
    // why return -1 when size_t unsigned?
    return -1;
  } // find

}; // struct find_index_space_from_id_u

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<size_t INDEX, typename TUPLE, size_t ID>
struct find_index_subspace_from_id_u {
  //--------------------------------------------------------------------------//
  //! Find the index corresponding to an entity type in the connectivities
  //! tuple - either from or to
  //!
  //! @tparam INDEX The current index in tuple.
  //! @tparam TUPLE The tuple type.
  //! @tparam ID Index space id to search for.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SUBSPACE = typename std::tuple_element<1, TUPLE_ELEMENT>::type;

    // Check match for ID and return if matched, recurse otherwise.
    return ID == INDEX_SUBSPACE::value
             ? INDEX - 1
             : find_index_subspace_from_id_u<INDEX - 1, TUPLE, ID>::find();
  } // find

}; // find_index_subspace_from_id_u

//----------------------------------------------------------------------------//
//! End recursion condition.
//----------------------------------------------------------------------------//

template<typename TUPLE, size_t ID>
struct find_index_subspace_from_id_u<0, TUPLE, ID> {

  //--------------------------------------------------------------------------//
  //! Search last tuple element.
  //--------------------------------------------------------------------------//

  static constexpr size_t find() {
    return -1;
  } // find

}; // struct find_index_subspace_from_id_u

//----------------------------------------------------------------------------//
//! Find all the entity index spaces within a certain domain
//!
//! @tparam I The current index in tuple.
//! @tparam MESH_TYPE The mesh type.
//! @tparam DOM  The domain to search.
//! @tparam Array  The deduced array type
//----------------------------------------------------------------------------//
//! @{
namespace detail {

template<size_t I,
  typename MESH_TYPE,
  size_t DOM,
  typename Array,
  std::enable_if_t<(I == MESH_TYPE::num_dimensions)> * = nullptr>
void
find_all_index_spaces_in_domain_u(Array && index_spaces) {
  std::forward<Array>(index_spaces)[I] = find_index_space_from_dimension_u<
    std::tuple_size<typename MESH_TYPE::entity_types>::value,
    typename MESH_TYPE::entity_types, I, DOM>::find();
}

template<size_t I,
  typename MESH_TYPE,
  size_t DOM,
  typename Array,
  typename = std::enable_if_t<(
    I<MESH_TYPE::num_dimensions && MESH_TYPE::num_dimensions> 0)>>
void
find_all_index_spaces_in_domain_u(Array && index_spaces) {
  std::forward<Array>(index_spaces)[I] = find_index_space_from_dimension_u<
    std::tuple_size<typename MESH_TYPE::entity_types>::value,
    typename MESH_TYPE::entity_types, I, DOM>::find();
  find_all_index_spaces_in_domain_u<I + 1, MESH_TYPE, DOM>(index_spaces);
}

} // namespace detail

// The main calling function
template<typename MESH_TYPE, size_t DOM>
auto
find_all_index_spaces_in_domain_u() {
  std::array<size_t, MESH_TYPE::num_dimensions + 1> index_spaces;
  detail::find_all_index_spaces_in_domain_u<0, MESH_TYPE, DOM>(index_spaces);
  return index_spaces;
}
//! @}

/*----------------------------------------------------------------------------*
 * Connectivity utilities.
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \struct compute_connectivity_u mesh_utils.h
//! \brief compute_connectivity_u provides static recursion to process
//! connectivity computation of mesh entity types.
//-----------------------------------------------------------------//
template<size_t FIND_DOM, size_t I, class TS>
struct compute_connectivity_u {
  //-----------------------------------------------------------------//
  //! Compute mesh connectivity for the given domain and tuple element.
  //!
  //!  @tparam FIND_DOM The domain to match.
  //!  @tparam I The current tuple index.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<class DOM>
  static int compute(DOM & mesh) {
    static constexpr size_t size = std::tuple_size<TS>::value;

    using T = typename std::tuple_element<size - I, TS>::type;
    using D1 = typename std::tuple_element<1, T>::type;
    using T1 = typename std::tuple_element<2, T>::type;
    using T2 = typename std::tuple_element<3, T>::type;

    if(D1::value == FIND_DOM) {
      mesh.template compute_connectivity<FIND_DOM, T1::dimension,
        T2::dimension>();
    }

    return compute_connectivity_u<FIND_DOM, I - 1, TS>::compute(mesh);
  } // compute

}; // struct compute_connectivity_u

//-----------------------------------------------------------------//
//! \struct compute_connectivity_u mesh_utils.h
//!  \brief compute_connectivity_u provides a specialization for
//!  the root recursion.
//-----------------------------------------------------------------//
template<size_t FIND_DOM, class TS>
struct compute_connectivity_u<FIND_DOM, 0, TS> {
  //-----------------------------------------------------------------//
  //! Terminate recursion.
  //!
  //!  @tparam FIND_DOM The domain to match.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<class DOM>
  static int compute(DOM &) {
    return 0;
  } // compute

}; // struct compute_connectivity_u

/*----------------------------------------------------------------------------*
 * Binding utilities.
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \struct compute_bindings_u mesh_utils.h
//! \brief compute_bindings_u provides static recursion to process
//! binding computation of mesh entity types.
//-----------------------------------------------------------------//
template<size_t FIND_DOM, size_t I, class TS>
struct compute_bindings_u {
  //-----------------------------------------------------------------//
  //! Compute mesh connectivity for the given domain and tuple element.
  //!
  //!  @tparam DOM The mesh type.
  //!  @tparam FIND_DOM The domain to match.
  //!  @tparam I The current tuple index.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<class DOM>
  static int compute(DOM & mesh) {
    static constexpr size_t size = std::tuple_size<TS>::value;

    // Get the indexed tuple
    using T = typename std::tuple_element<size - I, TS>::type;

    // Get domains and dimension
    using M1 = typename std::tuple_element<1, T>::type;
    using M2 = typename std::tuple_element<2, T>::type;
    using T1 = typename std::tuple_element<3, T>::type;
    using T2 = typename std::tuple_element<4, T>::type;

    if(M1::value == FIND_DOM) {
      mesh.template compute_bindings<M1::value, M2::value, T1::dimension,
        T2::dimension>();
    } // if

    return compute_bindings_u<FIND_DOM, I - 1, TS>::compute(mesh);
  } // compute

}; // struct compute_bindings_u

//-----------------------------------------------------------------//
//! \struct compute_bindings_u mesh_utils.h
//! \brief compute_bindings_u provides a specialization for
//! the root recursion.
//-----------------------------------------------------------------//
template<size_t FIND_DOM, class TS>
struct compute_bindings_u<FIND_DOM, 0, TS> {
  //-----------------------------------------------------------------//
  //! Terminate recursion.
  //!
  //! @tparam FIND_DOM The domain to match.
  //! @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<class DOM>
  static int compute(DOM &) {
    return 0;
  } // compute

}; // struct compute_bindings_u

template<typename T>
class mesh_graph_partition_u
{
public:
  using int_t = T;

  std::vector<int_t> offset;
  std::vector<int_t> index;
  std::vector<int_t> partition;
};

FLECSI_MEMBER_CHECKER(index_subspaces);

template<typename MESH_TYPE, bool HAS_SUBSPACES>
struct index_subspaces_tuple_u {
  using type = typename MESH_TYPE::index_subspaces;
};

template<typename MESH_TYPE>
struct index_subspaces_tuple_u<MESH_TYPE, false> {
  using type = std::tuple<>;
};

template<typename MESH_TYPE>
struct get_index_subspaces_u {
  using type = typename index_subspaces_tuple_u<MESH_TYPE,
    has_member_index_subspaces<MESH_TYPE>::value>::type;
};

template<typename MESH_TYPE>
struct num_index_subspaces_u {
  using type = typename get_index_subspaces_u<MESH_TYPE>::type;

  static constexpr size_t value = std::tuple_size<type>::value;
};

} // namespace topology
} // namespace flecsi
