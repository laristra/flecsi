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
namespace topo {
template<class>
struct ragged_topology; // defined in terms of field
}

namespace data {

/// A data accessor.
/// Pass a \c field_reference to a task that accepts an accessor.
/// \tparam L data layout
/// \tparam T data type
/// \tparam Priv access privileges
template<layout L, typename T, std::size_t Priv>
struct accessor;

/// A specialized accessor for changing the extent of dynamic layouts.
template<layout, class, std::size_t Priv>
struct mutator;

namespace detail {
template<class, layout>
struct field_base {};
template<class, layout, class Topo, typename Topo::index_space>
struct field_register;

// All field registration is ultimately defined in terms of raw fields.
template<class T, class Topo, typename Topo::index_space Space>
struct field_register<T, raw, Topo, Space> : field_info_t {
  explicit field_register(field_id_t i) : field_info_t{i, sizeof(T)} {
    run::context::instance().add_field_info<Topo, Space>(this);
  }
  field_register() : field_register(unique_fid_t::instance().next()) {}
  // Copying/moving is inadvisable because the context knows the address.
  field_register(const field_register &) = delete;
  field_register & operator=(const field_register &) = delete;
};
} // namespace detail

/// Identifies a field on a particular topology instance.
template<class Topo>
struct field_reference_t : convert_tag {
  using Topology = Topo;
  using topology_t = typename Topo::core;

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
/// Declare a task parameter as an \c accessor to use the field.
/// \tparam T data type (merely for type safety)
/// \tparam L data layout (similarly)
/// \tparam Space topology-relative index space
template<class T, layout L, class Topo, typename Topo::index_space Space>
struct field_reference : field_reference_t<Topo> {
  using Base = typename field_reference::field_reference_t; // TIP: dependent
  using value_type = T;
  static constexpr auto space = Space;

  using Base::Base;
  explicit field_reference(const Base & b) : Base(b) {}

  // We can't forward-declare partition, so just deduce these:
  template<class S>
  static auto & get_region(S & topo) {
    return topo.template get_region<Space>();
  }
  template<class S>
  auto & get_partition(S & topo) const {
    return topo.template get_partition<Space>(this->fid());
  }

  auto & get_region() const {
    return get_region(this->topology());
  }
  auto & get_partition() const {
    return get_partition(this->topology());
  }

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
struct field : data::detail::field_base<T, L> {
  using value_type = T;

  template<std::size_t Priv>
  using accessor1 = data::accessor<L, T, Priv>;
  template<std::size_t Priv>
  using mutator1 = data::mutator<L, T, Priv>;
  /// The accessor to use as a parameter to receive this sort of field.
  /// \tparam PP the appropriate number of privilege values
  template<partition_privilege_t... PP>
  using accessor = accessor1<privilege_pack<PP...>>;
  // The mutator to use as a parameter for this sort of field (usable only for
  // certain layouts).
  template<partition_privilege_t... PP>
  using mutator = mutator1<privilege_pack<PP...>>;

  template<class Topo, typename Topo::index_space S>
  using Register = data::detail::field_register<T, L, Topo, S>;
  template<class Topo, typename Topo::index_space S>
  using Reference = data::field_reference<T, L, Topo, S>;

  /// A field registration.
  /// \tparam Topo (specialized) topology type
  /// \tparam Space index space
  template<class Topo, typename Topo::index_space Space = Topo::default_space()>
  struct definition : Register<Topo, Space> {
    using Field = field;

    /// Return a reference to a field instance.
    /// \param t topology instance (must be allocated)
    auto operator()(data::topology_slot<Topo> & t) const {
      return (*this)(t.get());
    }
    Reference<Topo, Space> operator()(typename Topo::core & t) const {
      return {*this, t};
    }
  };
};

namespace data {
namespace detail {
template<class T>
struct field_base<T, dense> {
  using base_type = field<T, raw>;
};
template<class T>
struct field_base<T, single> {
  using base_type = field<T>;
};
template<class T>
struct field_base<T, ragged> {
  using base_type = field<T, raw>;
  using Offsets = field<std::size_t>;
};
template<class T>
struct field_base<T, sparse> {
  using base_type = field<std::pair<std::size_t, T>, ragged>;
};

// Many compilers incorrectly require the 'template' for a base class.
template<class T, layout L, class Topo, typename Topo::index_space Space>
struct field_register : field<T, L>::base_type::template Register<Topo, Space> {
  using Base = typename field<T, L>::base_type::template Register<Topo, Space>;
  using Base::Base;
};
template<class T, class Topo, typename Topo::index_space Space>
struct field_register<T, ragged, Topo, Space>
  : field<T, ragged>::base_type::template Register<topo::ragged_topology<Topo>,
      Space> {
  using Offsets = typename field<T, ragged>::Offsets;
  // We use the same field ID for the offsets:
  typename Offsets::template Register<Topo, Space> off{field_register::fid};

  typename Offsets::template Reference<Topo, Space> offsets(
    topology_slot<Topo> & t) const {
    return {off, t};
  }
};
} // namespace detail

template<class F, std::size_t Priv>
using field_accessor = // for convenience with decltype
  typename std::remove_reference_t<F>::Field::template accessor1<Priv>;
// Accessors that are always used with the same (internal) field can be
// automatically initialized with its ID rather than having to be serialized.
template<const auto & F, std::size_t Priv>
struct accessor_member : field_accessor<decltype(F), Priv> {
  accessor_member() : accessor_member::accessor(F.fid) {}
  using accessor_member::accessor::operator=; // for single

  template<class G>
  void topology_send(G && g) {
    // Using get_base() works around a GCC 9 bug that claims that the
    // inheritance of various accessor types is ambiguous.
    std::forward<G>(g)(get_base(), F);
  }
  template<class G, class S>
  void topology_send(G && g, S && s) { // s: topology -> subtopology
    std::forward<G>(g)(get_base(),
      [&s](auto & t) { return F(std::invoke(std::forward<S>(s), t.get())); });
  }

  typename accessor_member::accessor & get_base() {
    return *this;
  }
};

} // namespace data
} // namespace flecsi
