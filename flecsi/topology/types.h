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

#include <array>
#include <unordered_map>
#include <vector>

#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * struct typeify
 *----------------------------------------------------------------------------*/

template<typename T, T M>
struct typeify {
  static constexpr T value = M;
};

template<typename T, T M>
constexpr T typeify<T, M>::value;

template<size_t DIM>
using dimension_ = typeify<size_t, DIM>;

template<size_t DOM>
using domain_ = typeify<size_t, DOM>;

template<size_t IS>
using index_space_ = typeify<size_t, IS>;

template<size_t ISS>
using index_subspace_ = typeify<size_t, ISS>;

/*----------------------------------------------------------------------------*
 * Simple types
 *----------------------------------------------------------------------------*/

using id_vector_t = std::vector<utils::id_t>;
using connection_vector_t = std::vector<id_vector_t>;

// hash use for mapping in building topology connectivity
struct id_vector_hash_t {
  size_t operator()(const id_vector_t & v) const {
    size_t h = 0;
    for(utils::id_t id : v) {
      h |= static_cast<size_t>(id.local_id());
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// used when building the topology connectivities
using id_vector_map_t =
  std::unordered_map<id_vector_t, utils::id_t, id_vector_hash_t>;

// the second topology vector holds the offsets into to from dimension
using index_vector_t = std::vector<size_t>;

//-----------------------------------------------------------------//
//! \class entity_base_u types.h
//! \brief entity_base_u defines a base class that stores the raw info that
//! a topology needs, i.e: id and rank data
//!
//! \tparam N The number of domains.
//-----------------------------------------------------------------//

class entity_base_
{
public:
  using id_t = flecsi::utils::id_t;
};

template<size_t NUM_DOMAINS>
class entity_base_u : public entity_base_
{
public:
  ~entity_base_u() {}

  //-----------------------------------------------------------------//
  //! Return the id of this entity.
  //!
  //! \return The id of the entity.
  //-----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  const id_t & global_id() const {
    return id_;
  } // id

  FLECSI_INLINE_TARGET
  id_t & global_id() {
    return id_;
  } // id

  FLECSI_INLINE_TARGET
  size_t id() const {
    return id_.entity();
  } // id

  FLECSI_INLINE_TARGET
  uint16_t info() const {
    return global_id() >> 48;
  } // info

  //-----------------------------------------------------------------//
  //! Set the id of this entity.
  //-----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  void set_global_id(const id_t & id) {
    id_ = id;
  } // id

  /*!
   */
  FLECSI_INLINE_TARGET
  static constexpr size_t get_dim_(size_t meshDim, size_t dim) {
    return dim > meshDim ? meshDim : dim;
  } // get_dim_

protected:
  void set_info(uint16_t info) {
    id_ = (uint64_t(info) << 48) | global_id();
  } // set_info

private:
  id_t id_;

}; // class entity_base_u

//-----------------------------------------------------------------//
//! Define the vector type for storing entities.
//!
//! \tparam NUM_DOMAINS The number of domains.
//-----------------------------------------------------------------//
template<size_t NUM_DOMAINS>
using entity_vector_t = std::vector<entity_base_u<NUM_DOMAINS> *>;

} // namespace topology
} // namespace flecsi
