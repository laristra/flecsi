/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2020, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/backend.hh"
#include "flecsi/topology/base.hh"

namespace flecsi::data {

namespace detail {
#ifdef DOXYGEN // implemented per-backend
struct region {
  region(std::size_t, const fields &);
};

struct partition {
  template<class F> // F: int -> [start, end)
  partition(const region &, std::size_t, F);
  std::size_t colors() const;
};
#endif

template<class Topo, std::size_t Index = 0>
region
make_region(std::size_t n, storage_label_t sclass = dense) {
  return {n,
    runtime::context_t::instance().get_field_info_store<Topo, Index>(sclass)};
}

template<class Topo>
struct simple : region {
  using type = Topo;
  simple(std::size_t n = 1) : region(make_region<type>(n)) {}
};
} // namespace detail

template<typename>
struct topology_data;

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::global> : detail::simple<topology::global> {
  topology_data(const type::coloring &) {}
};

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::index> : detail::simple<topology::index>,
                                        detail::partition {
  topology_data(const type::coloring & coloring)
    : simple(coloring.size()),
      partition(*this, coloring.size(), [](std::size_t i) {
        return std::pair{i, i + 1};
      }) {}
};

template<>
struct topology_data<topology::canonical_base> {
  using type = topology::canonical_base;
  topology_data(const type::coloring &) {}
};

template<>
struct topology_data<topology::ntree_base> {
  using type = topology::ntree_base;
  topology_data(const type::coloring &) {}
};

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::unstructured_base> {
  using type = topology::unstructured_base;
  topology_data(const type::coloring &);

#if 0
  std::vector<base_data_t> entities;
  std::vector<base_data_t> adjacencies;
  std::vector<partition> exclusive;
  std::vector<partition> shared;
  std::vector<partition> ghost;
  std::vector<partition> ghost_owners;
#endif
};

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

template<>
struct topology_data<topology::structured_base> {
  using type = topology::structured_base;
  topology_data(const type::coloring &);
};

} // namespace flecsi::data
