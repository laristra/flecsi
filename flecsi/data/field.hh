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
#include "flecsi/run/backend.hh"
#include "flecsi/run/types.hh"
#include <flecsi/data/layout.hh>
#include <flecsi/data/privilege.hh>

namespace flecsi {
namespace data {

/// A data accessor.
/// \tparam L data layout
/// \tparam T data type
/// \tparam Priv access privileges
template<layout L, typename T, std::size_t Priv>
struct accessor;

template<class, layout, class Topo, typename Topo::index_space>
struct field_register;

template<class T, class Topo, typename Topo::index_space Space>
struct field_register<T, dense, Topo, Space> : field_info_t {
  field_register() : field_info_t{unique_fid_t::instance().next(), sizeof(T)} {
    run::context::instance().add_field_info<Topo, Space>(*this);
  }
  // Copying/moving is inadvisable because the context knows the address.
  field_register(const field_register &) = delete;
  field_register & operator=(const field_register &) = delete;
};

template<class T, class Topo, typename Topo::index_space Space>
struct field_register<T, singular, Topo, Space>
  : field_register<T, dense, Topo, Space> {};

/*!
  The field_reference_t type is used to reference fields. It adds a \em
  topology field to the \c reference_base to track the
  associated topology instance.
 */
template<class Topo>
struct field_reference_t : convert_tag {
  // The use of the slot allows creating field references statically, before
  // the topology object has been created.
  using topology_t = topology_slot<Topo>;

  field_reference_t(const field_info_t & info, topology_t & topology)
    : fid_(info.fid), topology_(&topology) {}

  field_id_t fid() const {
    return fid_;
  }
  topology_t & topology() const {
    return *topology_;
  } // topology_identifier

private:
  field_id_t fid_;
  topology_t * topology_;

}; // struct field_reference

/// A \c field_reference is a \c field_reference_t with more type information.
/// \tparam T data type (merely for type safety)
/// \tparam L data layout (similarly)
/// \tparam Space topology-relative index space
template<class T, layout L, class Topo, typename Topo::index_space Space>
struct field_reference : field_reference_t<Topo> {
  using Base = typename field_reference::field_reference_t; // TIP: dependent
  using value_type = T;

  using Base::Base;
  explicit field_reference(const Base & b) : Base(b) {}

  template<layout L2, class T2 = T> // TODO: allow only safe casts
  auto cast() const {
    return field_reference<T2, L2, Topo, Space>(*this);
  }
};

} // namespace data

/// Helper type to define and access fields.
/// \tparam T field value type: a trivially copyable type with no pointers
///   or references
/// \tparam L data layout
template<class T, data::layout L = data::dense>
struct field {
  template<std::size_t Priv>
  using accessor1 = data::accessor<L, T, Priv>;
  /// The accessor to use as a parameter to receive this sort of field.
  /// \tparam PP the appropriate number of privilege values
  template<partition_privilege_t... PP>
  using accessor = accessor1<privilege_pack<PP...>>;

  /// A field registration.
  /// \tparam Topo (specialized) topology type
  /// \tparam Space index space
  template<class Topo, typename Topo::index_space Space = Topo::default_space()>
  struct definition : data::field_register<T, L, Topo, Space> {
    using Field = field;

    /// Return a reference to a field instance.
    /// \tparam t topology instance (need not be allocated yet)
    data::field_reference<T, L, Topo, Space> operator()(
      data::topology_slot<Topo> & t) const {
      return {*this, t};
    }
  };
};

namespace data {
template<class F, std::size_t Priv>
using field_accessor =
  typename std::remove_reference_t<F>::Field::template accessor1<Priv>;
template<const auto & F, std::size_t Priv>
struct accessor_member : field_accessor<decltype(F), Priv> {
  accessor_member() : accessor_member::accessor(F.fid) {}
};

} // namespace data
} // namespace flecsi
