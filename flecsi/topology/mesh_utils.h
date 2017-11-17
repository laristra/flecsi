/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_mesh_utils_h
#define flecsi_topology_mesh_utils_h

#include <vector>

#include "flecsi/utils/common.h"

//----------------------------------------------------------------------------//
//! @file mesh_utils.h
//! @date Initial file creation: Dec 23, 2015
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * Tuple search utilities.
 *----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<
  size_t I,
  class T,
  size_t D,
  size_t M
>
struct find_entity__
{

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
  //! @tparam D The dimension to match.
  //! @tparam M The domain to match.
  //--------------------------------------------------------------------------//

  static
  constexpr
  size_t
  find()
  {
    // grab current types
    using E = typename std::tuple_element<I - 1, T>::type;
    using D1 = typename std::tuple_element<1, E>::type;
    using T1 = typename std::tuple_element<2, E>::type;

    // Check match for domain and dimension and return
    // index if matched or recurse if not matched.
    return D1::value == M && T1::dimension == D
        ? I
        : find_entity__<I - 1, T, D, M>::find();
  }

}; // find_entity__

//-----------------------------------------------------------------//
//! \struct find_entity__ mesh_utils.h
//! \brief find_entity__ provides a specialization for the root recursion.
//-----------------------------------------------------------------//
template<
  class T,
  size_t D,
  size_t M
>
struct find_entity__<0, T, D, M>
{
  //-----------------------------------------------------------------//
  //! Search last tuple element.
  //!
  //! @tparam T The tuple type.
  //! @tparam D The dimension to match.
  //! @tparam M The domain to match.
  //-----------------------------------------------------------------//
  static
  constexpr
  size_t
  find()
  {
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
template<
  class MT,
  size_t D,
  size_t M
>
struct find_entity_
{
  using entity_types = typename MT::entity_types;

  using pair_ = typename std::tuple_element<
      find_entity__<std::tuple_size<entity_types>::value, entity_types, D,
          M>::find() -
          1,
      entity_types>::type;

  //-----------------------------------------------------------------//
  //! Define the type returned by searching the tuple for matching
  //! dimension and domain.
  //-----------------------------------------------------------------//
  using type = typename std::tuple_element<2, pair_>::type;
};


template<
  size_t INDEX,
  class TUPLE,
  class ENTITY
>
struct find_index_space__
{

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

  static
  constexpr
  size_t
  find()
  {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<2, TUPLE_ELEMENT>::type;

    // Check match for domain and dimension and return
    // index if matched or recurse if not matched.
    return std::is_same<ELEMENT_ENTITY, ENTITY>::value ? INDEX_SPACE::value : 
      find_index_space__<INDEX - 1, TUPLE, ENTITY>::find();
  }

}; // find_index_space__

//-----------------------------------------------------------------//
//! \struct find_entity__ mesh_utils.h
//! \brief find_entity__ provides a specialization for the root recursion.
//-----------------------------------------------------------------//

template<
  class TUPLE,
  class ENTITY
>
struct find_index_space__<0, TUPLE, ENTITY>
{

  //-----------------------------------------------------------------//
  //! Search last tuple element.
  //!
  //!  @tparam T The tuple type.
  //!  @tparam D The dimension to match.
  //!  @tparam M The domain to match.
  //-----------------------------------------------------------------//
  static constexpr size_t find()
  {
    assert(false && "failed to find index space");
    return 1; 
  } // find_from

}; // struct find_index_space__


//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<
  size_t INDEX,
  typename TUPLE,
  size_t DIMENSION,
  size_t DOMAIN_
>
struct find_index_space_from_dimension__
{
  //--------------------------------------------------------------------------//
  //! Find the index corresponding to an entity type in the connectivities 
  //! tuple - either from or to
  //!
  //! @tparam I The current index in tuple.
  //! @tparam T The tuple type.
  //! @tparam E The entity type to find.
  //--------------------------------------------------------------------------//

  static
  constexpr
  size_t
  find()
  {
    // grab current types
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ENTITY_DOMAIN = typename std::tuple_element<1, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<2, TUPLE_ELEMENT>::type;

    // Check match for dimension and return if matched, recurse otherwise.
    return ( DIMENSION == ELEMENT_ENTITY::dimension &&
      DOMAIN_ == ENTITY_DOMAIN::value ) ? INDEX_SPACE::value :
      find_index_space_from_dimension__<
        INDEX - 1, TUPLE, DIMENSION, DOMAIN_
      >::find();
  } // find

}; // find_index_space_from_dimension__

//----------------------------------------------------------------------------//
//! End recursion condition.
//----------------------------------------------------------------------------//

template<
  typename TUPLE,
  size_t DIMENSION,
  size_t DOMAIN_
>
struct find_index_space_from_dimension__<0, TUPLE, DIMENSION, DOMAIN_>
{

  //--------------------------------------------------------------------------//
  //! Search last tuple element.
  //--------------------------------------------------------------------------//

  static
  constexpr
  size_t
  find()
  {
    return 1;
  } // find

}; // struct find_index_space_from_dimension__

/*----------------------------------------------------------------------------*
 * Connectivity utilities.
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \struct compute_connectivity__ mesh_utils.h
//! \brief compute_connectivity__ provides static recursion to process
//! connectivity computation of mesh entity types.
//-----------------------------------------------------------------//
template<
  size_t DM,
  size_t I,
 class TS
>
struct compute_connectivity__
{
  //-----------------------------------------------------------------//
  //! Compute mesh connectivity for the given domain and tuple element.
  //!
  //!  @tparam DM The domain to match.
  //!  @tparam I The current tuple index.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<
    class M
  >
  static
  int
  compute(
    M & mesh
  )
  {
    static constexpr size_t size = std::tuple_size<TS>::value;

    using T = typename std::tuple_element<size - I, TS>::type;
    using D1 = typename std::tuple_element<1, T>::type;
    using T1 = typename std::tuple_element<2, T>::type;
    using T2 = typename std::tuple_element<3, T>::type;

    if (D1::value == DM){
      mesh.template compute_connectivity<DM, T1::dimension, T2::dimension>();
    }

    return compute_connectivity__<DM, I - 1, TS>::compute(mesh);
  } // compute

}; // struct compute_connectivity__

//-----------------------------------------------------------------//
//! \struct compute_connectivity__ mesh_utils.h
//!  \brief compute_connectivity__ provides a specialization for
//!  the root recursion.
//-----------------------------------------------------------------//
template<
  size_t DM,
  class TS
>
struct compute_connectivity__<DM, 0, TS>
{
  //-----------------------------------------------------------------//
  //! Terminate recursion.
  //!
  //!  @tparam DM The domain to match.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<
    class M>
  static int compute(M &)
  {
    return 0;
  } // compute

}; // struct compute_connectivity__

/*----------------------------------------------------------------------------*
 * Binding utilities.
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \struct compute_bindings__ mesh_utils.h
//! \brief compute_bindings__ provides static recursion to process
//! binding computation of mesh entity types.
//-----------------------------------------------------------------//
template<
  size_t DM,
  size_t I,
  class TS
>
struct compute_bindings__
{
  //-----------------------------------------------------------------//
  //! Compute mesh connectivity for the given domain and tuple element.
  //!
  //!  @tparam M The mesh type.
  //!  @tparam DM The domain to match.
  //!  @tparam I The current tuple index.
  //!  @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<
    class M
  >
  static
  int
  compute(M & mesh)
  {
    static constexpr size_t size = std::tuple_size<TS>::value;

    // Get the indexed tuple
    using T = typename std::tuple_element<size - I, TS>::type;

    // Get domains and dimension
    using M1 = typename std::tuple_element<1, T>::type;
    using M2 = typename std::tuple_element<2, T>::type;
    using T1 = typename std::tuple_element<3, T>::type;
    using T2 = typename std::tuple_element<4, T>::type;

    if (M1::value == DM){
      mesh.template compute_bindings<M1::value, M2::value, T1::dimension,
          T2::dimension>();
    } // if

    return compute_bindings__<DM, I - 1, TS>::compute(mesh);
  } // compute

}; // struct compute_bindings__

//-----------------------------------------------------------------//
//! \struct compute_bindings__ mesh_utils.h
//! \brief compute_bindings__ provides a specialization for
//! the root recursion.
//-----------------------------------------------------------------//
template<
  size_t DM,
  class TS
>
struct compute_bindings__<DM, 0, TS>
{
  //-----------------------------------------------------------------//
  //! Terminate recursion.
  //!
  //! @tparam DM The domain to match.
  //! @tparam TS The tuple typel
  //-----------------------------------------------------------------//
  template<
    class M
  >
  static int compute(M &)
  {
    return 0;
  } // compute

}; // struct compute_bindings__

template<
  typename T
>
class mesh_graph_partition
{
public:
  using int_t = T;

  std::vector<int_t> offset;
  std::vector<int_t> index;
  std::vector<int_t> partition;
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_mesh_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
