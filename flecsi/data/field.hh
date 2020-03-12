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

#include "flecsi/data/topology_slot.hh"
#include <flecsi/data/layout.hh>
#include <flecsi/data/privilege.hh>
#include <flecsi/runtime/backend.hh>
#include <flecsi/runtime/types.hh>

namespace flecsi {
namespace data {

/// A data accessor.
/// \tparam L data layout
/// \tparam T data type
/// \tparam Priv access privileges
template<layout L, typename T, std::size_t Priv>
struct accessor;

template<class, layout, class, std::size_t>
struct field_register;

template<class T, class Topo, std::size_t Space>
struct field_register<T, dense, Topo, Space> : field_info_t {
  field_register() : field_info_t{unique_fid_t::instance().next(), sizeof(T)} {
    runtime::context_t::instance().add_field_info<Topo, Space>(*this);
  }
  // Copying/moving is inadvisable because the context knows the address.
  field_register(const field_register &) = delete;
  field_register & operator=(const field_register &) = delete;
};

template<class T, class Topo, std::size_t Space>
struct field_register<T, singular, Topo, Space>
  : field_register<T, dense, Topo, Space> {};

/*!
  The field_reference_t type is used to reference fields. It adds a \em
  topology field to the \c reference_base to track the
  associated topology instance.
 */
template<class Topo>
struct field_reference_t {
  // The use of the slot allows creating field references statically, before
  // the topology_data has been allocated.
  using topology_t = topology_slot<Topo>;

  field_reference_t(const field_info_t & info, const topology_t & topology)
    : fid_(info.fid), topology_(&topology) {}

  field_id_t fid() const {
    return fid_;
  }
  const topology_t & topology() const {
    return *topology_;
  } // topology_identifier

private:
  field_id_t fid_;
  const topology_t * topology_;

}; // struct field_reference

/// A \c field_reference is a \c field_reference_t with more type information.
/// \tparam T data type (merely for type safety)
/// \tparam Space topology-relative index space
template<class T, class Topo, topology::index_space_t<Topo> Space>
struct field_reference : field_reference_t<Topo> {
  using value_type = T;
  using field_reference_t<Topo>::field_reference_t;
};

/*!
  The field_member type provides a mechanism to define and register
  fundamental field types with the FleCSI runtime.

  @tparam DATA_TYPE     A compact type that will be defined at each index of
                        the associated topological index space. FleCSI defines
                        a compact type as either a P.O.D. type, or a
                        user-defined type that does not have external
                        references, i.e., sizeof for a compact type is equal to
                        the logical serialization of that type.
  @tparam L data layout
  @tparam TOPOLOGY_TYPE A specialization of a core FleCSI topology type.
  @tparam INDEX_SPACE   The index space on which to define the field.
 */

template<typename DATA_TYPE,
  layout L,
  typename TOPOLOGY_TYPE,
  topology::index_space_t<TOPOLOGY_TYPE> INDEX_SPACE =
    topology::default_space<TOPOLOGY_TYPE>>
struct field_member : field_register<DATA_TYPE, L, TOPOLOGY_TYPE, INDEX_SPACE> {
  using topology_reference_t = topology_slot<TOPOLOGY_TYPE>;

  template<size_t... PRIVILEGES>
  using accessor = accessor<L, DATA_TYPE, privilege_pack<PRIVILEGES...>::value>;

  /*!
    Return a reference to the field instance associated with the given topology
    reference.

    @param topology_reference A reference to a valid topology instance.
   */

  field_reference<DATA_TYPE, TOPOLOGY_TYPE, INDEX_SPACE> operator()(
    topology_reference_t const & topology_reference) const {

    return {*this, topology_reference};
  } // operator()
}; // struct field_member

} // namespace data
} // namespace flecsi
