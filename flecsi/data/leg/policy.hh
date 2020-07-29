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

static_assert(static_cast<Legion::coord_t>(logical_size) == logical_size,
  "logical_size too large for Legion");

namespace leg {
inline auto &
run() {
  return *Legion::Runtime::get_runtime();
}
inline auto
ctx() {
  return Legion::Runtime::get_context();
}

inline void
destroy(Legion::IndexSpace i) {
  run().destroy_index_space(ctx(), i);
}
// Legion seems to be buggy with destroying partitions:
inline void
destroy(Legion::IndexPartition) {}
inline void
destroy(Legion::FieldSpace f) {
  run().destroy_field_space(ctx(), f);
}
inline void
destroy(Legion::LogicalRegion r) {
  run().destroy_logical_region(ctx(), r);
}
inline void
destroy(Legion::LogicalPartition) {}

template<class T>
struct unique_handle {
  unique_handle() = default;
  unique_handle(T t) : h(t) {}
  unique_handle(unique_handle && u) noexcept : h(std::exchange(u.h, {})) {}
  ~unique_handle() {
    if(*this) // empty LogicalRegions, at least, cannot be deleted
      destroy(h);
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

using unique_index_space = unique_handle<Legion::IndexSpace>;
using unique_index_partition = unique_handle<Legion::IndexPartition>;
using unique_field_space = unique_handle<Legion::FieldSpace>;
using unique_logical_region = unique_handle<Legion::LogicalRegion>;
using unique_logical_partition = unique_handle<Legion::LogicalPartition>;

// NB: n=0 works because Legion interprets inverted ranges as empty.
inline Legion::coord_t
upper(std::size_t n) {
  return static_cast<Legion::coord_t>(n) - 1;
}

struct region {
  region(size2 s, const fields & fs)
    : index_space(run().create_index_space(ctx(),
        Legion::Rect<2>({0, 0},
          Legion::Point<2>(upper(s.first), upper(s.second))))),
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

  size2 size() const {
    const auto p = run().get_index_space_domain(index_space).hi();
    return size2(p[0] + 1, p[1] + 1);
  }
  template<topo::single_space>
  region & get_region() {
    return *this;
  }

  template<class D>
  void cleanup(field_id_t f, D d) {
    // We assume that creating the objects will be successful:
    destroy[f] = d;
  }

  unique_index_space index_space;
  unique_field_space field_space;
  unique_logical_region logical_region;

private:
  struct finalizer {
    finalizer() = default;
    template<class F>
    finalizer(F f) : f(std::move(f)) {}
    finalizer(finalizer && o) noexcept {
      f.swap(o.f); // guarantee o.f is empty
    }
    ~finalizer() {
      if(f)
        f();
    }
    finalizer & operator=(finalizer o) noexcept {
      f.swap(o.f);
      return *this;
    }

  private:
    std::function<void()> f;
  };

  std::map<field_id_t, finalizer> destroy;
};

struct partition {
  using row = Legion::Rect<2>;
  static row make_row(std::size_t i, std::size_t n) {
    const Legion::coord_t r = i;
    return {{r, 0}, {r, upper(n)}};
  }
  static std::size_t row_size(const row & r) {
    return r.hi[1] + 1;
  }

  explicit partition(const region & reg)
    : partition(reg, run().get_index_space_domain(reg.index_space).hi()) {}
  partition(const region & reg,
    const partition & src,
    field_id_t fid,
    completeness cpt = incomplete)
    : index_partition(part(reg.index_space, src, fid, cpt)),
      logical_partition(log(reg)) {}

  std::size_t colors() const {
    return run().get_index_space_domain(get_color_space()).get_volume();
  }

  unique_index_space color_space; // empty when made from another partition
  unique_index_partition index_partition;
  unique_logical_partition logical_partition;

  template<topo::single_space>
  const partition & get_partition() const {
    return *this;
  }

  void
  update(const partition & src, field_id_t fid, completeness cpt = incomplete) {
    auto & r = run();
    auto ip = part(r.get_parent_index_space(index_partition), src, fid, cpt);
    logical_partition = r.get_logical_partition(
      r.get_parent_logical_region(logical_partition), ip);
    index_partition = std::move(ip); // can't fail
  }

private:
  // The type-erased version assumes a square transformation matrix.
  partition(const region & reg, Legion::DomainPoint hi)
    : color_space(run().create_index_space(ctx(), Legion::Rect<1>(0, hi[0]))),
      index_partition(run().create_partition_by_restriction(
        ctx(),
        Legion::IndexSpaceT<2>(reg.index_space),
        Legion::IndexSpaceT<1>(color_space),
        [&] {
          Legion::Transform<2, 1> ret;
          ret.rows[0].x = 1;
          ret.rows[1].x = 0;
          return ret;
        }(),
        {{0, 0}, {0, hi[1]}},
        DISJOINT_COMPLETE_KIND)),
      logical_partition(log(reg)) {}

  // This is the same as color_space when that is non-empty.
  Legion::IndexSpace get_color_space() const {
    return run().get_index_partition_color_space_name(index_partition);
  }

  // We document that src must outlive this partitioning, although Legion is
  // said to support deleting its color space before our partition using it.
  unique_index_partition part(const Legion::IndexSpace & is,
    const partition & src,
    field_id_t fid,
    completeness cpt) {
    auto & r = run();
    return r.create_partition_by_image_range(ctx(),
      is,
      src.logical_partition,
      r.get_parent_logical_region(src.logical_partition),
      fid,
      src.get_color_space(),
      Legion::PartitionKind((cpt + 3) % 3 * 3));
  }
  unique_logical_partition log(const region & reg) const {
    return run().get_logical_partition(reg.logical_region, index_partition);
  }
};
} // namespace leg

using leg::region, leg::partition; // for backend-agnostic interface

} // namespace data
} // namespace flecsi
