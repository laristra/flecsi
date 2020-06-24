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
#endif

#include "flecsi/data/field.hh"
#include "flecsi/util/constant.hh"
#include "flecsi/util/typeify.hh"

#include <type_traits>

namespace flecsi {
namespace topo {

namespace detail {
template<class, class>
struct connect;
template<class P, class... VT>
struct connect<P, util::types<VT...>> {
  // FIXME: Use ragged instead of dense when it's functional.
  using type = util::key_tuple<util::key_type<VT::value,
    util::key_array<field<std::size_t, data::dense>::definition<P, VT::value>,
      typename VT::type>>...>;
};

template<class, std::size_t>
struct connect_access;
template<class... VT, std::size_t Priv>
struct connect_access<util::key_tuple<VT...>, Priv> {
  using type = util::key_tuple<util::key_type<VT::value,
    util::key_array<
      typename VT::type::value_type::Field::template accessor1<Priv>,
      typename VT::type::keys>>...>;
};
} // namespace detail

// Construct a "sparse matrix" of field definitions; the row is the source
// index space (which is enough to determine the field type) and the column is
// the destination.
template<class P>
using connect_t = typename detail::connect<P, typename P::connectivities>::type;

namespace detail {
template<class P, std::size_t Priv>
using connect_access_t = typename connect_access<connect_t<P>, Priv>::type;
}

// A parallel sparse matrix of accessors.
template<class P, std::size_t Priv>
struct connect_access : detail::connect_access_t<P, Priv> {
  // The argument type is just connect_t<P>, but we want the VT pack.
  // Prior to C++20, accessor_member can't refer to the subobjects of a
  // connect_t, so the accessors must be initialized externally.
  template<class... VT>
  connect_access(const util::key_tuple<VT...> & c)
    : detail::connect_access_t<P, Priv>(
        make_from<std::decay_t<decltype(this->template get<VT::value>())>>(
          c.template get<VT::value>())...) {}

private:
  // The .get<>s here and above just access the elements in order, of course.
  template<class T, class U, auto... VV>
  static T make_from(const util::key_array<U, util::constants<VV...>> & m) {
    return {typename T::value_type(m.template get<VV>().fid)...};
  }
};

// For either kind of sparse matrix:
template<class F, class T>
void
connect_visit(F && f, T && t) {
  std::forward<T>(t).apply([&](auto &... rr) {
    (
      [&](auto & r) {
        for(auto & x : r)
          std::forward<F>(f)(x);
      }(rr),
      ...);
  });
}

//----------------------------------------------------------------------------//
// Type creation utilities to create C++ types from size_t ids.
//----------------------------------------------------------------------------//

/*!
  Type to define different topological dimension types from size_t ids.

  @tparam TOPOLOGICAL_DIMENSION The size_t dimension.
 */

template<size_t TOPOLOGICAL_DIMENSION>
using topological_dimension = util::typeify<size_t, TOPOLOGICAL_DIMENSION>;

/*!
  Type to define different topological domain types from size_t ids.

  @tparam TOPOLOGICAL_DOMAIN The size_t domain.
 */

template<size_t TOPOLOGICAL_DOMAIN>
using topological_domain = util::typeify<size_t, TOPOLOGICAL_DOMAIN>;

/*!
  Type to define different index subspace types from size_t ids.

  @tparam INDEX_SUBSPACE The size_t index subspace.
 */

template<size_t INDEX_SUBSPACE>
using index_subspace = util::typeify<size_t, INDEX_SUBSPACE>;

//----------------------------------------------------------------------------//
// Simple Types.
//----------------------------------------------------------------------------//

using id_vector_t = std::vector<util::id_t>;
using connection_vector_t = std::vector<id_vector_t>;

// Hash type use for mappings in building topology connectivity

struct id_vector_hash_t {
  size_t operator()(const id_vector_t & v) const {
    size_t h = 0;
    for(util::id_t id : v) {
      h |= static_cast<size_t>(id.local_id());
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// Map type used when building the topology connectivities.

using id_vector_map_t =
  std::unordered_map<id_vector_t, util::id_t, id_vector_hash_t>;

// The second topology vector holds the offsets into to from dimension

using index_vector_t = std::vector<size_t>;
//-----------------------------------------------------------------//
//! \class entity_base types.h
//! \brief entity_base defines a base class that stores the raw info that
//! a topology needs, i.e: id and rank data
//!
//! \tparam N The number of domains.
//-----------------------------------------------------------------//

class entity_base_
{
public:
  using id_t = util::id_t;
};

template<size_t NUM_DOMAINS>
class entity_base : public entity_base_
{
public:
  ~entity_base() {}

  //-----------------------------------------------------------------//
  //! Return the id of this entity.
  //!
  //! \return The id of the entity.
  //-----------------------------------------------------------------//
  template<size_t DOM = 0>
  id_t global_id() const {
    return ids_[DOM];
  } // id

  id_t global_id(size_t domain) const {
    return ids_[domain];
  } // id

  template<size_t DOM = 0>
  size_t id() const {
    return ids_[DOM].entity();
  } // id

  size_t id(size_t domain) const {
    return ids_[domain].entity();
  } // id

  template<size_t DOM = 0>
  uint16_t info() const {
    return ids_[DOM] >> 48;
  } // info

  //-----------------------------------------------------------------//
  //! Set the id of this entity.
  //-----------------------------------------------------------------//
  template<size_t DOM = 0>
  void set_global_id(const id_t & id) {
    ids_[DOM] = id;
  } // id

  /*!
   */

  static constexpr size_t get_dim_(size_t meshDim, size_t dim) {
    return dim > meshDim ? meshDim : dim;
  } // get_dim_

protected:
  template<size_t DOM = 0>
  void set_info(uint16_t info) {
    ids_[DOM] = (uint64_t(info) << 48) | ids_[DOM];
  } // set_info

private:
  std::array<id_t, NUM_DOMAINS> ids_;

}; // class entity_base

//-----------------------------------------------------------------//
//! Define the vector type for storing entities.
//!
//! \tparam NUM_DOMAINS The number of domains.
//-----------------------------------------------------------------//
template<size_t NUM_DOMAINS>
using entity_vector_t = std::vector<entity_base<NUM_DOMAINS> *>;

} // namespace topo
} // namespace flecsi
