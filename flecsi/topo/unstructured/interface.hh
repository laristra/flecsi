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
#include "flecsi/flog.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/topo/core.hh"
#include "flecsi/topo/unstructured/types.hh"
#include "flecsi/topo/utility_types.hh"
#include "flecsi/util/color_map.hh"
#include "flecsi/util/dcrs.hh"

#include <map>
#include <utility>

namespace flecsi {
namespace topo {

namespace unstructured_impl {

/*
  This is a test implmentation to be used in developing and testing the
  unstructured topology type. It is not scalable, and should not be used
  for real problems!
 */

template<typename Definition>
inline util::dcrs
naive_coloring(Definition const & md,
  size_t entity_dimension,
  size_t thru_dimension,
  size_t process = run::context::instance().process(),
  size_t processes = run::context::instance().processes()) {

  /*
    This utility isn't really necessary here (since we are using colors =
    processes, but it is convenient nonetheless.
   */
  util::color_map cm(processes, processes, md.num_entities(entity_dimension));

  flog_assert(entity_dimension != thru_dimension,
    "thru dimension cannot equal entity dimension");

  /*
    The current implementation only supports graph creation of
    the highest dimensional entity type through lower dimensional types,
    e.g., cells through vertices, or edges.
    Support could be added for the inverse, e.g., vertices through edges,
    or cells. This assertion catches the unsupported case.
   */
  flog_assert(entity_dimension > thru_dimension,
    "current implementation does not support through dimensions"
    " greater than entity dimension");

  std::map<size_t, std::vector<size_t>> thru2entity;

  // This is not scalable!
  for(size_t e{0}; e < md.num_entities(entity_dimension); ++e) {
    for(auto v : md.entities(entity_dimension, 0, e)) {
      thru2entity[v].push_back(e);
    } // for
  } // for

  util::dcrs dcrs;
  dcrs.distribution = cm.distribution();

  const size_t color_offset = cm.index_offset(process, 0);
  const size_t color_indices = cm.indices(process, 0);

  std::map<size_t, std::vector<size_t>> entity2entities;

  for(size_t e{color_offset}; e < color_offset + color_indices; ++e) {
    std::map<size_t, size_t> thru_counts;

    for(auto v : md.entities(entity_dimension, 0, e)) {
      for(auto o : thru2entity[v]) {
        if(o != e) {
          thru_counts[o] += 1;
        } // if
      } // for
    } // for

    for(auto tc : thru_counts) {
      if(tc.second > thru_dimension) {
        entity2entities[e].push_back(tc.first);
        entity2entities[tc.first].push_back(e);
      } // if
    } // for
  } // for

  dcrs.offsets.push_back(0);
  for(size_t i{0}; i < color_indices; ++i) {
    auto e = dcrs.distribution[process] + i;

    std::set<size_t> set_indices;
    for(auto n : entity2entities[e]) {
      set_indices.insert(n);
    } // for

    for(auto i : set_indices) {
      dcrs.indices.push_back(i);
    } // for

    dcrs.offsets.push_back(dcrs.offsets[i] + set_indices.size());
  } // for

  flog_devel(info) << dcrs << std::endl;

  return dcrs;
} // naive_coloring

/*----------------------------------------------------------------------------*
  Dependency Closure.
 *----------------------------------------------------------------------------*/

template<typename Policy>
unstructured_base::coloring
closure(typename Policy::definition const & md,
  std::vector<size_t> const & primary) {
  (void)md;
  (void)primary;
} // closure

} // namespace unstructured_impl

/*----------------------------------------------------------------------------*
  Unstructured Topology.
 *----------------------------------------------------------------------------*/

template<typename Policy>
struct unstructured : unstructured_base {

  template<typename Definition>
  inline util::dcrs naive_coloring(Definition const & md,
    size_t entity_dimension,
    size_t thru_dimension,
    size_t process = run::context::instance().process(),
    size_t processes = run::context::instance().processes()) {
    return unstructured_impl::naive_coloring(
      md, entity_dimension, thru_dimension, process, processes);
  }

}; // struct unstructured

template<>
struct detail::base<unstructured> {
  using type = unstructured_base;
}; // struct detail::base<unstructured>

} // namespace topo
} // namespace flecsi
