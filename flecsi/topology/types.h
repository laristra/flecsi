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
    for (utils::id_t id : v) {
      h |= id.local_id();
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// used when building the topology connectivities
using id_vector_map_t =
    std::unordered_map<id_vector_t, utils::id_t, id_vector_hash_t>;

// the second topology vector holds the offsets into to from dimension
using index_vector_t = std::vector<size_t>;

} // namespace topology
} // namespace flecsi
