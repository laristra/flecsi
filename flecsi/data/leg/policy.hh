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

/*!  @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include "flecsi/run/backend.hh"
#include "flecsi/topo/core.hh" // single_space

#include <legion.h>

#include <unordered_map>

namespace flecsi {
namespace data {

namespace leg {
inline auto &
run() {
  return *Legion::Runtime::get_runtime();
}
inline auto
ctx() {
  return Legion::Runtime::get_context();
}

template<class T, void (Legion::Runtime::*D)(Legion::Context, T, bool)>
struct unique_handle {
  unique_handle() = default;
  unique_handle(T t) : h(t) {}
  unique_handle(unique_handle && u) noexcept : h(std::exchange(u.h, {})) {}
  ~unique_handle() {
    if(*this) // it's not clear whether empty handles can be deleted
      (run().*D)(ctx(), h, false);
  }
  unique_handle & operator=(unique_handle u) noexcept {
    std::swap(h, u.h);
    return *this;
  }
  explicit operator bool() {
    return h.exists();
  }
  operator T() const {
    return h;
  }

private:
  T h;
};

using unique_index_space =
  unique_handle<Legion::IndexSpace, &Legion::Runtime::destroy_index_space>;
// Legion seems to be buggy with destroying partitions:
using unique_index_partition = Legion::IndexPartition;
using unique_field_space =
  unique_handle<Legion::FieldSpace, &Legion::Runtime::destroy_field_space>;
using unique_logical_region = unique_handle<Legion::LogicalRegion,
  &Legion::Runtime::destroy_logical_region>;
using unique_logical_partition = Legion::LogicalPartition;

inline unique_index_space
index1(std::size_t n) {
  return run().create_index_space(ctx(), Legion::Rect<1>(0, n - 1));
}

struct region {
  region(std::size_t n, const fields & fs)
    : index_space(index1(n)),
      field_space([&fs] { // TIP: IIFE (q.v.) allows statements here
        auto & r = run();
        const auto c = ctx();
        unique_field_space ret = r.create_field_space(c);
        Legion::FieldAllocator allocator = r.create_field_allocator(c, ret);
        for(auto const & fi : fs)
          allocator.allocate_field(fi->type_size, fi->fid);
        return ret;
      }()),
      logical_region(
        run().create_logical_region(ctx(), index_space, field_space)) {}

  unique_index_space index_space;
  unique_field_space field_space;
  unique_logical_region logical_region;
};

struct partition {
  // TODO: support create_partition_by_image_range case
  template<class F>
  partition(const region & reg,
    std::size_t n,
    F f,
    disjointness dis = {},
    completeness cpt = {})
    : color_space(index1(n)),
      index_partition(run().create_partition_by_domain(
        ctx(),
        reg.index_space,
        [&] {
          std::map<Legion::DomainPoint, Legion::Domain> ret;
          for(std::size_t i = 0; i < n; ++i) {
            // NB: reg.index_space is assumed to be one-dimensional.
            const auto [b, e] = f(i);
            ret.try_emplace(i, Legion::Rect<1>(b, e - 1));
          }
          return ret;
        }(),
        color_space,
        true,
        Legion::PartitionKind((dis + 2) % 3 + (cpt + 3) % 3 * 3))),
      logical_partition(run().get_logical_partition(ctx(),
        reg.logical_region,
        index_partition)) {}

  std::size_t colors() const {
    return run().get_index_space_domain(color_space).get_volume();
  }

  unique_index_space color_space;
  unique_index_partition index_partition;
  unique_logical_partition logical_partition;

  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }
};
} // namespace leg

using leg::region, leg::partition; // for backend-agnostic interface

} // namespace data
} // namespace flecsi
