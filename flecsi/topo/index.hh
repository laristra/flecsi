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

template<class P>
struct size_category : index_base, data::partitioned<data::partition> {
  size_category(const coloring & c)
    : partitioned(data::make_region<P>({c.size(), 1})) {}
};
template<>
struct detail::base<size_category> {
  using type = index_base;
};

struct resize {
  explicit resize(std::size_t n) : size(n) {}

  auto operator()() {
    return field(size.get_slot());
  }
  auto & operator*() {
    return *size;
  }
  auto & operator*() const {
    return *size;
  }

  auto & get_slot() {
    return size.get_slot();
  }

  using Field = flecsi::field<data::partition::row, data::singular>;

private:
  // cslot can't be used, but is unneeded.
  struct topo : specialization<size_category, topo> {};
  data::anti_slot<topo> size;

public:
  static inline const Field::definition<topo> field;
};

// To control initialization order:
struct with_size {
  with_size(std::size_t n) : sizes(n) {}
  resize sizes;
};

// A partition with a field for dynamically resizing it.
struct repartition : with_size, data::partition {
private:
  static std::size_t k0(std::size_t) {
    return 0;
  }
  static constexpr auto zero = make_partial<k0>();

public:
  template<class F = decltype(zero)>
  repartition(const data::region & r, F f = zero);
  void resize() {
    update(*sizes, sizes().fid());
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
  data::anti_slot<ragged_topology<P>> ragged;
};

template<>
struct detail::base<ragged_category> {
  using type = ragged_base;
};

template<class P>
struct index_category : size_category<P>, with_ragged<P> {
  index_category(const index_base::coloring & c)
    : size_category<P>(c), with_ragged<P>(c.size()) {}
};
template<>
struct detail::base<index_category> {
  using type = index_base;
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
