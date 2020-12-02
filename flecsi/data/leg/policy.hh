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
#include "flecsi/run/leg/mapper.hh"
#include "flecsi/topo/core.hh" // single_space

#include <legion.h>

#include <unordered_map>

namespace flecsi {
namespace data {

enum disjointness { compute = 0, disjoint = 1, aliased = 2 };
constexpr int
partitionKind(disjointness dis, completeness cpt) {
  return (dis + 2) % 3 + 3 * cpt;
}

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

// Legion uses a number of "handle" types that are non-owning identifiers for
// inaccessible objects maintained by the runtime.  By wrapping their deletion
// functions in a uniform interface, we can use normal RAII hereafter.

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

  bool poll_discard(field_id_t f) {
    return discard.erase(f);
  }

  unique_index_space index_space;
  unique_field_space field_space;
  unique_logical_region logical_region;

protected:
  void vacuous(field_id_t f) {
    discard.insert(f);
  }

private:
  std::set<field_id_t> discard;
};

struct partition {
  using row = Legion::Rect<2>;
  using point = Legion::Point<2>;

  static point make_point(std::size_t i, std::size_t j) {
    return point(i, j);
  }

  static row make_row(std::size_t i, std::size_t n) {
    const Legion::coord_t r = i;
    return {{r, 0}, {r, upper(n)}};
  }

  static row make_row(std::size_t i, subrow n) {
    const Legion::coord_t r = i;
    const Legion::coord_t ln = n.first;
    return {{r, ln}, {r, upper(n.second)}};
  }

  static std::size_t row_size(const row & r) {
    return r.hi[1] - r.lo[1] + 1;
  }

  static constexpr struct BuildByImage_tag {
  } buildByImage_tag = {};

  explicit partition(const region & reg)
    : partition(reg, run().get_index_space_domain(reg.index_space).hi()) {}
  partition(const region & reg,
    const partition & src,
    field_id_t fid,
    disjointness dis = disjoint,
    completeness cpt = incomplete)
    : index_partition(part(reg.index_space, src, fid, dis, cpt)),
      logical_partition(log(reg)) {}

  partition(const region & reg,
    const partition & src,
    field_id_t fid,
    BuildByImage_tag,
    disjointness dis = compute,
    completeness cpt = incomplete)
    : index_partition(part<false>(reg.index_space, src, fid, dis, cpt)),
      logical_partition(log(reg)) {}

  std::size_t colors() const {
    return run().get_index_space_domain(get_color_space()).get_volume();
  }

  unique_index_space color_space; // empty when made from another partition
  unique_index_partition index_partition;
  unique_logical_partition logical_partition;

  template<topo::single_space>
  const partition & get_partition(field_id_t) const {
    return *this;
  }

  void update(const partition & src,
    field_id_t fid,
    disjointness dis = disjoint,
    completeness cpt = incomplete) {
    auto & r = run();
    auto ip =
      part(r.get_parent_index_space(index_partition), src, fid, dis, cpt);
    logical_partition = r.get_logical_partition(
      r.get_parent_logical_region(logical_partition), ip);
    index_partition = std::move(ip); // can't fail
  }

  // This is the same as color_space when that is non-empty.
  Legion::IndexSpace get_color_space() const {
    return run().get_index_partition_color_space_name(index_partition);
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

  // We document that src must outlive this partitioning, although Legion is
  // said to support deleting its color space before our partition using it.
  template<bool R = true>
  unique_index_partition part(const Legion::IndexSpace & is,
    const partition & src,
    field_id_t fid,
    disjointness dis,
    completeness cpt) {
    auto & r = run();

    Legion::Color part_color =
      r.get_index_partition_color(ctx(), src.index_partition);

    auto tag = flecsi::run::tag_index_partition(part_color);

    return [&r](auto &&... aa) {
      return R ? r.create_partition_by_image_range(
                   std::forward<decltype(aa)>(aa)...)
               : r.create_partition_by_image(std::forward<decltype(aa)>(aa)...);
    }(ctx(),
             is,
             src.logical_partition,
             r.get_parent_logical_region(src.logical_partition),
             fid,
             src.get_color_space(),
             Legion::PartitionKind(partitionKind(dis, cpt)),
             LEGION_AUTO_GENERATE_ID,
             0,
             tag);
  }

  unique_logical_partition log(const region & reg) const {
    return run().get_logical_partition(reg.logical_region, index_partition);
  }
};

} // namespace leg

using region_base = leg::region;
using leg::partition;

inline void
launch_copy(const region_base & reg,
  const partition & src_partition,
  const partition & dest_partition,
  const field_id_t & data_fid,
  const field_id_t & ptr_fid) {

  Legion::IndexCopyLauncher cl_(src_partition.get_color_space());
  Legion::LogicalRegion lreg = reg.logical_region;
  Legion::LogicalPartition lp_source = src_partition.logical_partition;
  Legion::LogicalPartition lp_dest = dest_partition.logical_partition;
  Legion::RegionRequirement rr_source(
    lp_source, 0 /*projection ID*/, READ_ONLY, EXCLUSIVE, lreg);
  Legion::RegionRequirement rr_dest(
    lp_dest, 0 /*projection ID*/, WRITE_ONLY, EXCLUSIVE, lreg);
  Legion::RegionRequirement rr_pos(
    lp_dest, 0 /*projection ID*/, READ_ONLY, EXCLUSIVE, lreg);

  rr_source.add_field(data_fid);
  rr_dest.add_field(data_fid);
  rr_pos.add_field(ptr_fid);

  cl_.add_copy_requirements(rr_source, rr_dest);
  cl_.add_src_indirect_field(ptr_fid, rr_pos);
  assert(!cl_.src_indirect_is_range[0]);
  leg::run().issue_copy_operation(leg::ctx(), cl_);
}

} // namespace data
} // namespace flecsi
