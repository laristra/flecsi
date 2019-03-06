/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 7, 2017
//----------------------------------------------------------------------------//

#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/coloring/adjacency_types.h>
#include <flecsi/coloring/coloring_types.h>
#include <flecsi/coloring/index_coloring.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/internal_index_space.h>
#include <flecsi/execution/legion/helper.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/utils/offset.h>

clog_register_tag(legion_data);

namespace flecsi {
namespace data {

/*!
  This class provides a centralized mechanism for creating Legion
  index spaces, field spaces, partitions, and logical regions using
  FleCSI data representation schemes: distributed 2d BLIS, global, and
  color, and mesh topology adjacency data. This class is instantiated
  to create such structures, but it is destroyed afterwards, then this
  data is retained in the execution context.

   @ingroup data
 */

class legion_data_t
{
public:
  using coloring_info_t = coloring::coloring_info_t;

  using adjacency_info_t = coloring::adjacency_info_t;

  using index_subspace_info_t = execution::context_t::index_subspace_info_t;

  using coloring_info_map_t = std::unordered_map<size_t, coloring_info_t>;

  using indexed_coloring_info_map_t = std::map<size_t, coloring_info_map_t>;

  using sparse_index_space_info_t =
    execution::context_t::sparse_index_space_info_t;

  using sparse_index_space_info_map_t =
    std::map<size_t, sparse_index_space_info_t>;

  /*!
    Collects all of the information needed to represent a FleCSI BLIS
    index space.
   */
  struct index_space_t {
    size_t index_space_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition access_partition;
    Legion::IndexPartition color_partition;
    Legion::IndexPartition primary_partition;
    Legion::IndexPartition exclusive_partition;
    Legion::IndexPartition shared_partition;
    Legion::IndexPartition ghost_partition;
    size_t total_num_entities;
    bool has_sparse_fields = false;
  };

  struct sparse_index_space_t {
    size_t index_space_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition access_partition;
    Legion::IndexPartition color_partition;
    Legion::IndexPartition primary_partition;
    Legion::IndexPartition exclusive_partition;
    Legion::IndexPartition shared_partition;
    Legion::IndexPartition ghost_partition;
    Legion::IndexPartition all_shared_partition;
    size_t exclusive_reserve;
    size_t max_entries_per_index;
    size_t max_shared_ghost;
    size_t color_size;
  };

  struct sparse_metadata_t {
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition index_partition;
    Legion::LogicalPartition logical_partition;
  };

  /*!
    Collects all of the information needed to represent an index subspace.
   */
  struct index_subspace_t {
    size_t index_subspace_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition index_partition;
    size_t capacity;
    Legion::FieldID fid;
  };

  /*!
    Collects all of the information needed to represent a mesh topology
    adjacency index space.
   */
  struct adjacency_t {
    size_t index_space_id;
    size_t from_index_space_id;
    size_t to_index_space_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition index_partition;
    size_t max_conn_size;
  };

  legion_data_t(Legion::Context ctx,
    Legion::Runtime * runtime,
    size_t num_colors)
    : ctx_(ctx), runtime_(runtime), h(runtime, ctx), num_colors_(num_colors),
      color_bounds_(0, num_colors_ - 1),
      color_domain_(Legion::Domain::from_rect<1>(color_bounds_)) {}

  ~legion_data_t() {
    for(auto & itr : index_space_map_) {
      index_space_t & is = itr.second;

      runtime_->destroy_index_partition(ctx_, is.color_partition);
      runtime_->destroy_index_space(ctx_, is.index_space);
      runtime_->destroy_field_space(ctx_, is.field_space);
      runtime_->destroy_logical_region(ctx_, is.logical_region);
    }
    runtime_->destroy_index_space(ctx_, global_index_space_.index_space);
    runtime_->destroy_field_space(ctx_, global_index_space_.field_space);
    runtime_->destroy_logical_region(ctx_, global_index_space_.logical_region);

    runtime_->destroy_index_space(ctx_, color_index_space_.index_space);
    runtime_->destroy_field_space(ctx_, color_index_space_.field_space);
    runtime_->destroy_logical_region(ctx_, color_index_space_.logical_region);
  }

  void init_global_handles() {
    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    // Create global index space
    const size_t index_space_id = execution::internal_index_space::global_is;

    index_spaces_.insert(index_space_id);

    global_index_space_.index_space_id = index_space_id;

    LegionRuntime::Arrays::Rect<1> bounds(
      LegionRuntime::Arrays::Point<1>(0), LegionRuntime::Arrays::Point<1>(1));

    Domain dom(Domain::from_rect<1>(bounds));

    global_index_space_.index_space = runtime_->create_index_space(ctx_, dom);
    attach_name(global_index_space_, global_index_space_.index_space,
      "global index space");

    // Read user + FleCSI registered field spaces
    global_index_space_.field_space = runtime_->create_field_space(ctx_);

    attach_name(global_index_space_, global_index_space_.field_space,
      "global field space");

    FieldAllocator allocator =
      runtime_->create_field_allocator(ctx_, global_index_space_.field_space);

    using field_info_t = context_t::field_info_t;

    for(const field_info_t & fi : context.registered_fields()) {
      if(fi.storage_class == global) {
        allocator.allocate_field(fi.size, fi.fid);
      } // if
    } // for

    global_index_space_.logical_region = runtime_->create_logical_region(
      ctx_, global_index_space_.index_space, global_index_space_.field_space);
    attach_name(global_index_space_, global_index_space_.logical_region,
      "global logical region");

  } // init_global_handles

  void init_from_coloring_info_map(
    const indexed_coloring_info_map_t & indexed_coloring_info_map,
    const sparse_index_space_info_map_t & sparse_info_map,
    bool has_sparse_fields) {
    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    for(auto & idx_space : indexed_coloring_info_map) {
      auto itr = sparse_info_map.find(idx_space.first);
      const sparse_index_space_info_t * sparse_info;

      if(has_sparse_fields && itr != sparse_info_map.end()) {
        sparse_info = &itr->second;
      }
      else {
        sparse_info = nullptr;
      }

      add_index_space(idx_space.first, idx_space.second, sparse_info);
    }

    // Create color index space
    {
      const size_t index_space_id = execution::internal_index_space::color_is;

      index_spaces_.insert(index_space_id);

      color_index_space_.index_space_id = index_space_id;

      color_index_space_.index_space =
        runtime_->create_index_space(ctx_, color_domain_);
      attach_name(color_index_space_, color_index_space_.index_space,
        "color index space");

      // Read user + FleCSI registered field spaces
      color_index_space_.field_space = runtime_->create_field_space(ctx_);

      attach_name(color_index_space_, color_index_space_.field_space,
        "color field space");
    } // scope

  } // init_from_coloring_info_map

  /*!
    Create a new BLIS index space.
   */
  void add_index_space(size_t index_space_id,
    const coloring_info_map_t & coloring_info_map,
    const sparse_index_space_info_t * sparse_info) {
    using namespace std;

    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    index_space_t is;
    is.index_space_id = index_space_id;

    // Create expanded IndexSpace
    index_spaces_.insert(index_space_id);

    // Determine max size of a color partition
    is.total_num_entities = 0;
    for(auto color_idx : coloring_info_map) {
      {
        clog_tag_guard(legion_data);
        clog(trace) << "index: " << index_space_id
                    << " color: " << color_idx.first << " " << color_idx.second
                    << std::endl;
      } // scope

      is.total_num_entities = std::max(is.total_num_entities,
        color_idx.second.exclusive + color_idx.second.shared +
          color_idx.second.ghost);
    } // for color_idx

    {
      clog_tag_guard(legion_data);
      clog(trace) << "total_num_entities " << is.total_num_entities
                  << std::endl;
    } // scope

    // Create expanded index space
    LegionRuntime::Arrays::Rect<2> expanded_bounds =
      LegionRuntime::Arrays::Rect<2>(LegionRuntime::Arrays::Point<2>::ZEROES(),
        make_point(num_colors_, is.total_num_entities));

    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));

    is.index_space = runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(is, is.index_space, "expanded index space");

    // Read user + FleCSI registered field spaces
    is.field_space = runtime_->create_field_space(ctx_);

    attach_name(is, is.field_space, "expanded field space");

    if(sparse_info) {
      is.has_sparse_fields = true;

      sparse_index_space_t sis;
      // TODO: need to formalize this index space offset scheme
      sis.index_space_id = index_space_id + 8192;

      sis.max_shared_ghost = 0;

      for(auto color_idx : coloring_info_map) {
        sis.max_shared_ghost = std::max(sis.max_shared_ghost,
          color_idx.second.shared + color_idx.second.ghost);
      }

      sis.exclusive_reserve = sparse_info->exclusive_reserve;
      sis.max_entries_per_index = sparse_info->max_entries_per_index;

      sis.color_size = sis.exclusive_reserve +
                       sis.max_shared_ghost * sis.max_entries_per_index;

      // Create expanded index space
      LegionRuntime::Arrays::Rect<2> expanded_bounds =
        LegionRuntime::Arrays::Rect<2>(
          LegionRuntime::Arrays::Point<2>::ZEROES(),
          make_point(num_colors_, sis.color_size));

      Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));

      sis.index_space = runtime_->create_index_space(ctx_, expanded_dom);
      attach_name(sis, sis.index_space, "expanded sparse index space");

      // Read user + FleCSI registered field spaces
      sis.field_space = runtime_->create_field_space(ctx_);

      attach_name(sis, sis.field_space, "expanded sparse field space");

      sparse_index_space_map_[index_space_id] = std::move(sis);
    }

    index_space_map_[index_space_id] = std::move(is);
  }

  /*!
    Create a mesh topology adjacency index space.
   */
  void add_adjacency(const adjacency_info_t & adjacency_info) {
    using namespace std;

    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    using field_info_t = context_t::field_info_t;

    adjacency_t c;

    clog_assert(
      adjacencies_.find(adjacency_info.index_space) == adjacencies_.end(),
      "adjacency exists");

    adjacencies_.insert(adjacency_info.index_space);

    c.index_space_id = adjacency_info.index_space;
    c.from_index_space_id = adjacency_info.from_index_space;
    c.to_index_space_id = adjacency_info.to_index_space;

    auto fitr = index_space_map_.find(c.from_index_space_id);
    clog_assert(fitr != index_space_map_.end(), "invalid from index space");
    const index_space_t & fi = fitr->second;

    auto titr = index_space_map_.find(c.to_index_space_id);
    clog_assert(titr != index_space_map_.end(), "invalid to index space");
    const index_space_t & ti = titr->second;

    auto citr = adjacency_map_.find(c.index_space_id);
    clog_assert(citr == adjacency_map_.end(), "invalid adjacency info");
    const adjacency_t & ci = citr->second;

    c.max_conn_size = fi.total_num_entities * ti.total_num_entities;

    // Create expanded index space
    LegionRuntime::Arrays::Rect<2> expanded_bounds =
      LegionRuntime::Arrays::Rect<2>(LegionRuntime::Arrays::Point<2>::ZEROES(),
        make_point(num_colors_, c.max_conn_size));

    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));
    c.index_space = runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(c, c.index_space, "expanded index space");

    // Read user + FleCSI registered field spaces
    c.field_space = runtime_->create_field_space(ctx_);

    FieldAllocator allocator =
      runtime_->create_field_allocator(ctx_, c.field_space);

    for(const field_info_t & fi : context.registered_fields()) {
      if(fi.index_space == c.index_space_id) {
        allocator.allocate_field(fi.size, fi.fid);
      }
    }

    attach_name(c, c.field_space, "expanded field space");

    c.logical_region =
      runtime_->create_logical_region(ctx_, c.index_space, c.field_space);
    attach_name(c, c.logical_region, "expanded logical region");

    clog_assert(adjacency_info.color_sizes.size() == num_colors_,
      "mismatch in color sizes");

    DomainColoring color_partitioning;
    for(size_t color = 0; color < num_colors_; ++color) {
      LegionRuntime::Arrays::Rect<2> subrect(make_point(color, 0),
        make_point(color, adjacency_info.color_sizes[color] - 1));

      color_partitioning[color] = Domain::from_rect<2>(subrect);
    }

    c.index_partition = runtime_->create_index_partition(ctx_, c.index_space,
      color_domain_, color_partitioning, true /*disjoint*/);
    attach_name(c, c.index_partition, "color partitioning");

    adjacency_map_.emplace(adjacency_info.index_space, std::move(c));
  }

  void add_index_subspace(const index_subspace_info_t & info) {
    using namespace std;

    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    index_subspace_t is;
    is.index_subspace_id = info.index_subspace;
    is.capacity = info.capacity;

    // Create expanded index space
    LegionRuntime::Arrays::Rect<2> expanded_bounds =
      LegionRuntime::Arrays::Rect<2>(LegionRuntime::Arrays::Point<2>::ZEROES(),
        make_point(num_colors_, is.capacity));

    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));

    is.index_space = runtime_->create_index_space(ctx_, expanded_dom);
    is.field_space = runtime_->create_field_space(ctx_);

    using field_info_t = context_t::field_info_t;

    FieldAllocator allocator =
      runtime_->create_field_allocator(ctx_, is.field_space);

    for(const field_info_t & fi : context.registered_fields()) {
      if(fi.storage_class == data::subspace &&
         fi.index_space == is.index_subspace_id) {
        allocator.allocate_field(fi.size, fi.fid);
        is.fid = fi.fid;
      }
    }

    is.logical_region =
      runtime_->create_logical_region(ctx_, is.index_space, is.field_space);

    DomainColoring color_partitioning;
    for(size_t color = 0; color < num_colors_; ++color) {
      LegionRuntime::Arrays::Rect<2> subrect(
        make_point(color, 0), make_point(color, is.capacity - 1));

      color_partitioning[color] = Domain::from_rect<2>(subrect);
    }

    is.index_partition = runtime_->create_index_partition(ctx_, is.index_space,
      color_domain_, color_partitioning, true /*disjoint*/);

    index_subspace_map_.emplace(info.index_subspace, std::move(is));
  }

  /*!
    After gathering all of the necessary information specified in the methods
    above this method is finally called to assemble and allocate fields spaces
    and logical regions for BLIS index spaces.
   */
  void finalize(const indexed_coloring_info_map_t & indexed_coloring_info_map) {
    using namespace std;

    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    for(auto & itr : index_space_map_) {
      index_space_t & is = itr.second;

      sparse_index_space_t * sis;

      auto sitr = sparse_index_space_map_.find(is.index_space_id);
      if(sitr != sparse_index_space_map_.end()) {
        sis = &sitr->second;
      }
      else {
        sis = nullptr;
      }

      auto citr = indexed_coloring_info_map.find(is.index_space_id);
      clog_assert(
        citr != indexed_coloring_info_map.end(), "invalid index space");
      const coloring_info_map_t & coloring_info_map = citr->second;

      FieldAllocator allocator =
        runtime_->create_field_allocator(ctx_, is.field_space);

      auto ghost_owner_pos_fid = FieldID(internal_field::ghost_owner_pos);

      allocator.allocate_field(
        sizeof(LegionRuntime::Arrays::Point<2>), ghost_owner_pos_fid);

      using field_info_t = context_t::field_info_t;

      for(const field_info_t & fi : context.registered_fields()) {
        switch(fi.storage_class) {
          case global:
          case color:
          case local:
          case subspace:
            break;
          case ragged:
          case sparse:
            if(fi.index_space == is.index_space_id) {
              if(utils::hash::is_internal(fi.key)) {
                allocator.allocate_field(fi.size, fi.fid);
              }
              else {
                // this is a sparse offset
                allocator.allocate_field(sizeof(utils::offset_t), fi.fid);
              }
            }
            break;
          default:
            if(fi.index_space == is.index_space_id) {
              allocator.allocate_field(fi.size, fi.fid);
            }
            break;
        }
      } // for

      is.logical_region =
        runtime_->create_logical_region(ctx_, is.index_space, is.field_space);
      attach_name(is, is.logical_region, "expanded logical region");

      // Partition expanded IndexSpace color-wise & create associated
      DomainColoring color_partitioning;
      MultiDomainColoring access_partitioning;
      MultiDomainColoring owner_partitioning;
      DomainColoring primary_partitioning;
      DomainColoring exclusive_partitioning;
      DomainColoring shared_partitioning;
      DomainColoring ghost_partitioning;

      for(int color = 0; color < num_colors_; color++) {
        auto citr = coloring_info_map.find(color);
        clog_assert(citr != coloring_info_map.end(), "invalid color info");
        const coloring_info_t & color_info = citr->second;

        LegionRuntime::Arrays::Rect<2> subrect(make_point(color, 0),
          make_point(color,
            color_info.exclusive + color_info.shared + color_info.ghost - 1));
        color_partitioning[color] = Domain::from_rect<2>(subrect);
        LegionRuntime::Arrays::Rect<2> primary_rect(make_point(color, 0),
          make_point(color, color_info.exclusive + color_info.shared - 1));
        primary_partitioning[color] = Domain::from_rect<2>(primary_rect);
        access_partitioning[PRIMARY_ACCESS].insert(
          Domain::from_rect<2>(primary_rect));

        LegionRuntime::Arrays::Rect<2> exclusive_rect(
          make_point(color, 0), make_point(color, color_info.exclusive - 1));
        exclusive_partitioning[color] = Domain::from_rect<2>(exclusive_rect);
        owner_partitioning[EXCLUSIVE_OWNER].insert(
          Domain::from_rect<2>(exclusive_rect));

        LegionRuntime::Arrays::Rect<2> shared_rect(
          make_point(color, color_info.exclusive),
          make_point(color, color_info.exclusive + color_info.shared - 1));
        shared_partitioning[color] = Domain::from_rect<2>(shared_rect);
        owner_partitioning[SHARED_OWNER].insert(
          Domain::from_rect<2>(shared_rect));

        LegionRuntime::Arrays::Rect<2> ghost_rect(
          make_point(color, color_info.exclusive + color_info.shared),
          make_point(color,
            color_info.exclusive + color_info.shared + color_info.ghost - 1));
        ghost_partitioning[color] = Domain::from_rect<2>(ghost_rect);
        access_partitioning[GHOST_ACCESS].insert(
          Domain::from_rect<2>(ghost_rect));
      } // for_color

      { // scope

        is.color_partition = runtime_->create_index_partition(ctx_,
          is.index_space, color_domain_, color_partitioning, true /*disjoint*/);
        attach_name(is, is.color_partition, "color partitioning");

        LegionRuntime::Arrays::Rect<1> access_bounds(
          PRIMARY_ACCESS, GHOST_ACCESS);
        Legion::Domain access_domain(
          Legion::Domain::from_rect<1>(access_bounds));

        is.access_partition =
          runtime_->create_index_partition(ctx_, is.index_space, access_domain,
            access_partitioning, true /*disjoint*/);
        attach_name(is, is.access_partition, "access partitioning");

        LogicalPartition access_lp = runtime_->get_logical_partition(
          ctx_, is.logical_region, is.access_partition);

        LogicalRegion primary_region = runtime_->get_logical_subregion_by_color(
          ctx_, access_lp, PRIMARY_ACCESS);

        IndexSpace ghost_is =
          runtime_->get_logical_subregion_by_color(
                    ctx_, access_lp, GHOST_ACCESS)
            .get_index_space();
        is.ghost_partition = runtime_->create_index_partition(
          ctx_, ghost_is, color_domain_, ghost_partitioning, true /*disjoint*/);
        attach_name(is, is.ghost_partition, "ghost partitioning");

        LegionRuntime::Arrays::Rect<1> owner_bounds(
          EXCLUSIVE_OWNER, SHARED_OWNER);
        Legion::Domain owner_domain(Legion::Domain::from_rect<1>(owner_bounds));

        IndexPartition owner_partition = runtime_->create_index_partition(ctx_,
          primary_region.get_index_space(), owner_domain, owner_partitioning,
          true /*disjoint*/);
        attach_name(is, owner_partition, "owner partitioning");

        LogicalPartition owner_lp = runtime_->get_logical_partition(
          ctx_, primary_region, owner_partition);

        IndexSpace primary_is = primary_region.get_index_space();
        is.primary_partition = runtime_->create_index_partition(ctx_,
          primary_is, color_domain_, primary_partitioning, true /*disjoint*/);
        attach_name(is, is.primary_partition, "primary partitioning");

        IndexSpace exclusive_is =
          runtime_
            ->get_logical_subregion_by_color(ctx_, owner_lp, EXCLUSIVE_OWNER)
            .get_index_space();
        is.exclusive_partition =
          runtime_->create_index_partition(ctx_, exclusive_is, color_domain_,
            exclusive_partitioning, true /*disjoint*/);
        attach_name(is, is.exclusive_partition, "exclusive partitioning");

        IndexSpace shared_is =
          runtime_->get_logical_subregion_by_color(ctx_, owner_lp, SHARED_OWNER)
            .get_index_space();
        is.shared_partition = runtime_->create_index_partition(ctx_, shared_is,
          color_domain_, shared_partitioning, true /*disjoint*/);
        attach_name(is, is.shared_partition, "shared partitioning");
      } // scope
      if(sis) {
        FieldAllocator allocator =
          runtime_->create_field_allocator(ctx_, sis->field_space);

        allocator.allocate_field(
          sizeof(LegionRuntime::Arrays::Point<2>), ghost_owner_pos_fid);

        for(const field_info_t & fi : context.registered_fields()) {
          switch(fi.storage_class) {
            case ragged:
              if(fi.index_space == is.index_space_id) {
                if(!utils::hash::is_internal(fi.key)) {
                  allocator.allocate_field(fi.size, fi.fid);
                }
              }
              break;
            case sparse:
              // TODO:  We should be able to remove "+ sizeof(size_t)"
              //        below, if we track down some other places where
              //        it's also added manually, and remove those too.
              if(fi.index_space == is.index_space_id) {
                if(!utils::hash::is_internal(fi.key)) {
                  allocator.allocate_field(fi.size + sizeof(size_t), fi.fid);
                }
              }
              break;
            default:
              break;
          }
        } // for

        sis->logical_region = runtime_->create_logical_region(
          ctx_, sis->index_space, sis->field_space);
        attach_name(
          *sis, sis->logical_region, "sparse expanded logical region");

        DomainColoring sis_color_partitioning;
        MultiDomainColoring sis_access_partitioning;
        MultiDomainColoring sis_owner_partitioning;
        DomainColoring sis_primary_partitioning;
        DomainColoring sis_exclusive_partitioning;
        DomainColoring sis_shared_partitioning;
        DomainColoring sis_ghost_partitioning;
        DomainColoring sis_all_shared_partitioning;

        const sparse_index_space_info_t * sparse_info;

        size_t max_shared = 0;

        for(int color = 0; color < num_colors_; color++) {
          auto citr = coloring_info_map.find(color);
          clog_assert(citr != coloring_info_map.end(), "invalid color info");
          const coloring_info_t & coloring_info = citr->second;

          size_t shared_size =
            coloring_info.shared * sis->max_entries_per_index;

          max_shared = std::max(shared_size, max_shared);
        }

        for(int color = 0; color < num_colors_; color++) {
          auto citr = coloring_info_map.find(color);
          clog_assert(citr != coloring_info_map.end(), "invalid color info");
          const coloring_info_t & coloring_info = citr->second;

          size_t shared_size =
            coloring_info.shared * sis->max_entries_per_index;

          size_t ghost_size = coloring_info.ghost * sis->max_entries_per_index;

          LegionRuntime::Arrays::Rect<2> subrect(make_point(color, 0),
            make_point(
              color, sis->exclusive_reserve + shared_size + ghost_size - 1));
          sis_color_partitioning[color] = Domain::from_rect<2>(subrect);

          LegionRuntime::Arrays::Rect<2> primary_rect(make_point(color, 0),
            make_point(color, sis->exclusive_reserve + shared_size - 1));

          sis_primary_partitioning[color] = Domain::from_rect<2>(primary_rect);
          sis_access_partitioning[PRIMARY_ACCESS].insert(
            Domain::from_rect<2>(primary_rect));

          LegionRuntime::Arrays::Rect<2> exclusive_rect(make_point(color, 0),
            make_point(color, sis->exclusive_reserve - 1));
          sis_exclusive_partitioning[color] =
            Domain::from_rect<2>(exclusive_rect);
          sis_owner_partitioning[EXCLUSIVE_OWNER].insert(
            Domain::from_rect<2>(exclusive_rect));

          LegionRuntime::Arrays::Rect<2> shared_rect(
            make_point(color, sis->exclusive_reserve),
            make_point(color, sis->exclusive_reserve + shared_size - 1));
          sis_shared_partitioning[color] = Domain::from_rect<2>(shared_rect);
          sis_owner_partitioning[SHARED_OWNER].insert(
            Domain::from_rect<2>(shared_rect));

          LegionRuntime::Arrays::Rect<2> ghost_rect(
            make_point(color, sis->exclusive_reserve + shared_size),
            make_point(
              color, sis->exclusive_reserve + shared_size + ghost_size - 1));
          sis_ghost_partitioning[color] = Domain::from_rect<2>(ghost_rect);
          sis_access_partitioning[GHOST_ACCESS].insert(
            Domain::from_rect<2>(ghost_rect));

          LegionRuntime::Arrays::Rect<2> all_shared_rect(
            make_point(0, sis->exclusive_reserve),
            make_point(num_colors_, sis->exclusive_reserve + max_shared - 1));
          sis_all_shared_partitioning[color] =
            Domain::from_rect<2>(all_shared_rect);
        }

        sis->color_partition =
          runtime_->create_index_partition(ctx_, sis->index_space,
            color_domain_, sis_color_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->color_partition, "color partitioning");

        LegionRuntime::Arrays::Rect<1> sis_access_bounds(
          PRIMARY_ACCESS, GHOST_ACCESS);
        Legion::Domain sis_access_domain(
          Legion::Domain::from_rect<1>(sis_access_bounds));

        sis->access_partition =
          runtime_->create_index_partition(ctx_, sis->index_space,
            sis_access_domain, sis_access_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->access_partition, "access partitioning");

        LogicalPartition sis_access_lp = runtime_->get_logical_partition(
          ctx_, sis->logical_region, sis->access_partition);

        LogicalRegion sis_primary_region =
          runtime_->get_logical_subregion_by_color(
            ctx_, sis_access_lp, PRIMARY_ACCESS);

        IndexSpace sis_ghost_is =
          runtime_
            ->get_logical_subregion_by_color(ctx_, sis_access_lp, GHOST_ACCESS)
            .get_index_space();

        sis->ghost_partition =
          runtime_->create_index_partition(ctx_, sis_ghost_is, color_domain_,
            sis_ghost_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->ghost_partition, "ghost partitioning");

        LegionRuntime::Arrays::Rect<1> sis_owner_bounds(
          EXCLUSIVE_OWNER, SHARED_OWNER);
        Legion::Domain sis_owner_domain(
          Legion::Domain::from_rect<1>(sis_owner_bounds));

        IndexPartition sis_owner_partition = runtime_->create_index_partition(
          ctx_, sis_primary_region.get_index_space(), sis_owner_domain,
          sis_owner_partitioning, true /*disjoint*/);
        attach_name(*sis, sis_owner_partition, "owner partitioning");

        LogicalPartition sis_owner_lp = runtime_->get_logical_partition(
          ctx_, sis_primary_region, sis_owner_partition);

        IndexSpace sis_primary_is = sis_primary_region.get_index_space();
        sis->primary_partition =
          runtime_->create_index_partition(ctx_, sis_primary_is, color_domain_,
            sis_primary_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->primary_partition, "primary partitioning");

        IndexSpace sis_exclusive_is = runtime_
                                        ->get_logical_subregion_by_color(
                                          ctx_, sis_owner_lp, EXCLUSIVE_OWNER)
                                        .get_index_space();
        sis->exclusive_partition =
          runtime_->create_index_partition(ctx_, sis_exclusive_is,
            color_domain_, sis_exclusive_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->exclusive_partition, "exclusive partitioning");

        IndexSpace sis_shared_is =
          runtime_
            ->get_logical_subregion_by_color(ctx_, sis_owner_lp, SHARED_OWNER)
            .get_index_space();
        sis->shared_partition =
          runtime_->create_index_partition(ctx_, sis_shared_is, color_domain_,
            sis_shared_partitioning, true /*disjoint*/);
        attach_name(*sis, sis->shared_partition, "shared partitioning");

        IndexSpace sis_all_shared_is =
          runtime_
            ->get_logical_subregion_by_color(ctx_, sis_owner_lp, SHARED_OWNER)
            .get_index_space();
        sis->all_shared_partition =
          runtime_->create_index_partition(ctx_, sis->index_space,
            color_domain_, sis_all_shared_partitioning, false /*alias*/);
        attach_name(*sis, sis->shared_partition, "shared partitioning");
      }
    }

    // create logical regions for color_index_space_
    {
      FieldAllocator allocator =
        runtime_->create_field_allocator(ctx_, color_index_space_.field_space);

      using field_info_t = context_t::field_info_t;

      for(const field_info_t & fi : context.registered_fields()) {
        if(fi.storage_class == color) {
          allocator.allocate_field(fi.size, fi.fid);
        } // if
      } // for
      color_index_space_.logical_region = runtime_->create_logical_region(
        ctx_, color_index_space_.index_space, color_index_space_.field_space);
      attach_name(color_index_space_, color_index_space_.logical_region,
        "color logical region");

      LegionRuntime::Arrays::Blockify<1> coloring(1);
      color_index_space_.color_partition = runtime_->create_index_partition(
        ctx_, color_index_space_.index_space, coloring);

      attach_name(color_index_space_, color_index_space_.color_partition,
        "color partitioning");
    } // scope

  } // finalize

  const index_space_t & index_space(size_t index_space_id) const {
    auto itr = index_space_map_.find(index_space_id);
    clog_assert(itr != index_space_map_.end(), "invalid index space");
    return itr->second;
  }

  const sparse_index_space_t & sparse_index_space(
    size_t sparse_index_space_id) const {
    auto itr = sparse_index_space_map_.find(sparse_index_space_id);
    clog_assert(
      itr != sparse_index_space_map_.end(), "invalid sparse index space");
    return itr->second;
  }

  const std::set<size_t> & index_spaces() const {
    return index_spaces_;
  }

  const adjacency_t & adjacency(size_t index_space_id) const {
    auto itr = adjacency_map_.find(index_space_id);
    clog_assert(itr != adjacency_map_.end(), "invalid adjacency");
    return itr->second;
  }

  const std::set<size_t> & adjacencies() const {
    return adjacencies_;
  }

  const Legion::Domain & color_domain() const {
    return color_domain_;
  }

  const std::unordered_map<size_t, adjacency_t> & adjacency_map() const {
    return adjacency_map_;
  }

  const std::map<size_t, index_subspace_t> & index_subspace_map() const {
    return index_subspace_map_;
  }

  index_space_t global_index_space() {
    return global_index_space_;
  }

  index_space_t color_index_space() {
    return color_index_space_;
  }

  void init_sparse_metadata() {
    using namespace std;

    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    LegionRuntime::Arrays::Rect<2> expanded_bounds =
      LegionRuntime::Arrays::Rect<2>(
        LegionRuntime::Arrays::Point<2>::ZEROES(), make_point(num_colors_, 1));

    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));

    sparse_metadata_.index_space =
      runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(sparse_metadata_, sparse_metadata_.index_space,
      "expanded sparse metadata index space");

    // Read user + FleCSI registered field spaces
    sparse_metadata_.field_space = runtime_->create_field_space(ctx_);

    attach_name(sparse_metadata_, sparse_metadata_.field_space,
      "expanded sparse metadata field space");

    FieldAllocator allocator =
      runtime_->create_field_allocator(ctx_, sparse_metadata_.field_space);

    for(const context_t::field_info_t & fi : context.registered_fields()) {
      if(fi.storage_class == sparse || fi.storage_class == ragged) {
        allocator.allocate_field(
          sizeof(context_t::sparse_field_data_t), fi.fid);
      } // if
    } // for

    sparse_metadata_.logical_region = runtime_->create_logical_region(
      ctx_, sparse_metadata_.index_space, sparse_metadata_.field_space);

    attach_name(sparse_metadata_, sparse_metadata_.logical_region,
      "expanded sparse metadata logical region");

    DomainColoring color_partitioning;
    for(int color = 0; color < num_colors_; color++) {
      LegionRuntime::Arrays::Rect<2> subrect(
        make_point(color, 0), make_point(color, 1));

      color_partitioning[color] = Domain::from_rect<2>(subrect);
    }

    sparse_metadata_.index_partition =
      runtime_->create_index_partition(ctx_, sparse_metadata_.index_space,
        color_domain_, color_partitioning, true /*disjoint*/);

    attach_name(sparse_metadata_, sparse_metadata_.index_partition,
      "sparse metadata color partitioning");

    sparse_metadata_.logical_partition = runtime_->get_logical_partition(
      ctx_, sparse_metadata_.logical_region, sparse_metadata_.index_partition);
  }

  const sparse_metadata_t & sparse_metadata() {
    return sparse_metadata_;
  }

private:
  Legion::Context ctx_;

  Legion::HighLevelRuntime * runtime_;

  execution::legion_helper h;

  size_t num_colors_;

  LegionRuntime::Arrays::Rect<1> color_bounds_;

  Legion::Domain color_domain_;

  std::set<size_t> index_spaces_;

  // key: index space
  std::unordered_map<size_t, index_space_t> index_space_map_;

  // key: index space
  std::unordered_map<size_t, sparse_index_space_t> sparse_index_space_map_;

  sparse_metadata_t sparse_metadata_;

  // key: index space
  std::unordered_map<size_t, adjacency_t> adjacency_map_;

  // key: index space
  std::map<size_t, index_subspace_t> index_subspace_map_;

  std::set<size_t> adjacencies_;

  index_space_t global_index_space_;

  index_space_t color_index_space_;

  template<class T>
  void attach_name(const index_space_t & is, T & x, const char * label) {
    std::stringstream sstr;
    sstr << label << " " << is.index_space_id;
    runtime_->attach_name(x, sstr.str().c_str());
  }

  template<class T>
  void attach_name(const sparse_index_space_t & is, T & x, const char * label) {
    std::stringstream sstr;
    sstr << label << " " << is.index_space_id;
    runtime_->attach_name(x, sstr.str().c_str());
  }

  template<class T>
  void attach_name(const adjacency_t & c, T & x, const char * label) {
    std::stringstream sstr;
    sstr << label << " " << c.from_index_space_id << "->"
         << c.to_index_space_id;
    runtime_->attach_name(x, sstr.str().c_str());
  }

  template<class T>
  void attach_name(const sparse_metadata_t &, T & x, const char * label) {
    std::stringstream sstr;
    runtime_->attach_name(x, label);
  }

}; // struct legion_data_t

} // namespace data

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
