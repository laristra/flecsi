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

#include "flecsi/data/accessor.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/exec/launch.hh"
#include "flecsi/topo/core.hh"

namespace flecsi {
namespace topo {

struct index_base {
  struct coloring {
    coloring(size_t size) : size_(size) {}

    size_t size() const {
      return size_;
    }

  private:
    size_t size_;
  };
};

// A topology with one index point per color.
// Suitable for the singular layout but not the ragged layout, which is
// implemented in terms of this topology.
template<class P>
struct color_category : index_base, data::partitioned<data::partition> {
  color_category(const coloring & c)
    : partitioned(data::make_region<P>({c.size(), 1})) {}
};
template<>
struct detail::base<color_category> {
  using type = index_base;
};

// A subtopology for storing/updating row sizes of a partition.
struct resize : specialization<color_category, resize> {
  // cslot is useless without a color function, but we don't need it.
  using Field = flecsi::field<data::partition::row, data::singular>;
  static const Field::definition<resize> field;
  using accessor = data::accessor_member<field, privilege_pack<ro>>;
};
// Now that resize is complete:
inline const resize::Field::definition<resize> resize::field;

// To control initialization order:
struct with_size {
  with_size(std::size_t n) : sz(n) {}
  auto sizes() {
    return resize::field(sz);
  }
  resize::core sz;
};

namespace zero {
inline std::size_t
function(std::size_t) {
  return 0;
}
inline constexpr auto partial = make_partial<function>();
} // namespace zero

// A partition with a field for dynamically resizing it.
struct repartition : with_size, data::partition {
  // Construct a partition with an initial size.
  // f is passed as a task argument, so it must be serializable;
  // consider using make_partial.
  template<class F = decltype(zero::partial)>
  repartition(const data::region & r, F f = zero::partial);
  void resize() { // apply sizes stored in the field
    update(sz, resize::field.fid);
  }

private:
  template<class F>
  static void fill(resize::Field::accessor<wo> a, const F & f) {
    const auto i = run::context::instance().color();
    a = data::partition::make_row(i, f(i));
  }
};

using repartitioned = data::partitioned<repartition>;

template<class T, typename T::index_space S = T::default_space(), class F>
repartitioned
make_repartitioned(std::size_t r, F f) {
  return {data::make_region<T, S>({r, data::logical_size}), std::move(f)};
}

// Stores the flattened elements of the ragged fields on an index space.
struct ragged_partitioned : data::region {
  ragged_partitioned(std::size_t r, const data::fields & fs)
    : region({r, data::logical_size}, fs) {
    for(const auto & fi : fs)
      part.try_emplace(fi->fid, *this);
  }
  repartition & operator[](field_id_t i) {
    return part.at(i);
  }
  const repartition & operator[](field_id_t i) const {
    return part.at(i);
  }

private:
  std::map<field_id_t, repartition> part;
};

struct ragged_base {
  using coloring = std::size_t;
};
template<class P>
struct ragged_category : ragged_base {
  using index_spaces = typename P::index_spaces;

  ragged_category(coloring c) : part(make_partitions(c, index_spaces())) {}

  std::size_t colors() const {
    return part.front().size().first;
  }

  template<typename P::index_space S>
  const repartition & get_partition(field_id_t i) const {
    return part.template get<S>()[i];
  }
  template<typename P::index_space S>
  repartition & get_partition(field_id_t i) {
    return part.template get<S>()[i];
  }

  // These can't just be default template arguments, since they would be
  // instantiated even if unused.
  const repartition & get_partition(field_id_t i) const {
    return const_cast<ragged_category &>(*this).get_partition(i);
  }
  repartition & get_partition(field_id_t i) {
    return get_partition<P::default_space()>(i);
  }

private:
  template<auto... VV>
  static util::key_array<ragged_partitioned, util::constants<VV...>>
  make_partitions(std::size_t n,
    util::constants<VV...> /* index_spaces, to deduce a pack */
  ) {
    return {{ragged_partitioned(
      n, run::context::instance().get_field_info_store<P, VV>())...}};
  }
  util::key_array<ragged_partitioned, index_spaces> part;
};
template<class T>
struct ragged_topology : specialization<ragged_category, ragged_topology<T>> {
  using index_space = typename T::index_space;
  using index_spaces = typename T::index_spaces;
};

template<class P>
struct with_ragged { // for interface consistency
  with_ragged(std::size_t n) : ragged(n) {}

  // Extend a offsets fields to define empty rows for the suffix.
  template<typename P::index_space, class F = decltype(zero::partial)>
  void extend_offsets(
    F = zero::partial); // serializable function from color to old size

  typename ragged_topology<P>::core ragged;

private:
  template<class F>
  static void extend(field<std::size_t, data::raw>::accessor<rw> a, F old) {
    const auto s = a.span();
    const std::size_t i = old(run::context::instance().color());
    // A dense field will be default-constructed on first use, but this
    // function means to support the case where it was resized.
    std::uninitialized_fill(s.begin() + i, s.end(), i ? s.back() : 0);
  }
};

template<>
struct detail::base<ragged_category> {
  using type = ragged_base;
};

// The user-facing variant of the color category supports ragged fields.
template<class P>
struct index_category : color_category<P>, with_ragged<P> {
  index_category(const index_base::coloring & c)
    : color_category<P>(c), with_ragged<P>(c.size()) {
    this->template extend_offsets<elements>();
  }
};
template<>
struct detail::base<index_category> {
  using type = index_base;
};

// A subtopology for holding topology-specific metadata per color.
template<class P>
struct meta_topology : specialization<index_category, meta_topology<P>> {};

template<class P>
struct with_meta { // for interface consistency
  with_meta(std::size_t n) : meta(n) {}
  typename meta_topology<P>::core meta;
};

/*!
  The \c index type allows users to register data on an
  arbitrarily-sized set of indices that have an implicit one-to-one coloring.

  @ingroup topology
 */
struct index : specialization<index_category, index> {
  static coloring color(size_t size) {
    return {size};
  } // color

}; // struct index

} // namespace topo
} // namespace flecsi
