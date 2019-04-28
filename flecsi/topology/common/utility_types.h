/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/utils/common.h>
#endif

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Type creation utilities to create C++ types from size_t ids.
//----------------------------------------------------------------------------//

/*!
  Type to define different topological dimension types from size_t ids.

  @tparam TOPOLOGICAL_DIMENSION The size_t dimension.
 */

template<size_t TOPOLOGICAL_DIMENSION>
using topological_dimension = utils::typeify<size_t, TOPOLOGICAL_DIMENSION>;

/*!
  Type to define different topological domain types from size_t ids.

  @tparam TOPOLOGICAL_DOMAIN The size_t domain.
 */

template<size_t TOPOLOGICAL_DOMAIN>
using topological_domain = utils::typeify<size_t, TOPOLOGICAL_DOMAIN>;

/*!
  Type to define different index space types from size_t ids.

  @tparam INDEX_SPACE The size_t index space.
 */

template<size_t INDEX_SPACE>
using index_space = utils::typeify<size_t, INDEX_SPACE>;

/*!
  Type to define different index subspace types from size_t ids.

  @tparam INDEX_SUBSPACE The size_t index subspace.
 */

template<size_t INDEX_SUBSPACE>
using index_subspace = utils::typeify<size_t, INDEX_SUBSPACE>;

//----------------------------------------------------------------------------//
// Simple Types.
//----------------------------------------------------------------------------//

using id_vector_t = std::vector<utils::id_t>;
using connection_vector_t = std::vector<id_vector_t>;

// Hash type use for mappings in building topology connectivity

struct id_vector_hash_t {
  size_t operator()(const id_vector_t & v) const {
    size_t h = 0;
    for(utils::id_t id : v) {
      h |= static_cast<size_t>(id.local_id());
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// Map type used when building the topology connectivities.

using id_vector_map_t =
  std::unordered_map<id_vector_t, utils::id_t, id_vector_hash_t>;

// The second topology vector holds the offsets into to from dimension

using index_vector_t = std::vector<size_t>;

} // namespace topology
} // namespace flecsi
