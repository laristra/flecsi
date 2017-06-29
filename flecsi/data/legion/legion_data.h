/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_legion_data_h
#define flecsi_legion_legion_data_h

#include <cassert>
#include <legion.h>
#include <map>
#include <unordered_map>
#include <vector>

#include "flecsi/execution/context.h"
#include "flecsi/coloring/index_coloring.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/coloring/adjacency_types.h"
#include "flecsi/execution/legion/helper.h"
#include "flecsi/execution/legion/legion_tasks.h"

///
/// \file legion/legion_data.h
/// \date Initial file creation: Jun 7, 2017
///

namespace flecsi{

namespace data{

class legion_data_t{
public:

  using coloring_info_t = coloring::coloring_info_t;

  using adjacency_info_t = coloring::adjacency_info_t;

  using coloring_info_map_t = std::unordered_map<size_t, coloring_info_t>;

  using indexed_coloring_info_map_t = std::map<size_t, coloring_info_map_t>;

  struct index_space_t{
    size_t index_space_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition index_partition;
    size_t total_num_entities;
  };

  struct adjacency_t{
    size_t index_space_id;
    size_t from_index_space_id;
    size_t to_index_space_id;
    Legion::IndexSpace index_space;
    Legion::FieldSpace field_space;
    Legion::LogicalRegion logical_region;
    Legion::IndexPartition index_partition;
    size_t max_conn_size;
  };

  legion_data_t(
    Legion::Context ctx,
    Legion::Runtime* runtime,
    size_t num_colors
  )
  : ctx_(ctx),
    runtime_(runtime),
    h(runtime, ctx),
    num_colors_(num_colors),
    color_bounds_(0, num_colors_ - 1),
    color_domain_(Legion::Domain::from_rect<1>(color_bounds_))
  {
  }

  ~legion_data_t()
  {
    for(auto& itr : index_space_map_){
      index_space_t& is = itr.second;

      runtime_->destroy_index_partition(ctx_, is.index_partition);
      runtime_->destroy_index_space(ctx_, is.index_space);
      runtime_->destroy_field_space(ctx_, is.field_space);
      runtime_->destroy_logical_region(ctx_, is.logical_region);
    }
  }

  void
  init_from_coloring_info_map(
    const indexed_coloring_info_map_t& indexed_coloring_info_map
  )
  {
    for(auto& idx_space : indexed_coloring_info_map){
      add_index_space(idx_space.first, idx_space.second);
    }
  }

  void
  add_index_space(
    size_t index_space_id,
    const coloring_info_map_t& coloring_info_map
  )
  {
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
    for(auto color_idx : coloring_info_map){
      clog(trace) << "index: " << index_space_id << " color: " << 
        color_idx.first << " " << color_idx.second << std::endl;
      
      is.total_num_entities = std::max(is.total_num_entities,
        color_idx.second.exclusive + color_idx.second.shared + 
        color_idx.second.ghost);
    } // for color_idx
    
    clog(trace) << "total_num_entities " << is.total_num_entities << std::endl;

    // Create expanded index space
    Rect<2> expanded_bounds = 
      Rect<2>(Point<2>::ZEROES(), make_point(num_colors_, is.total_num_entities));
    
    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));
    
    is.index_space = runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(is, is.index_space, "expanded index space");

    // Read user + FleCSI registered field spaces
    is.field_space = runtime_->create_field_space(ctx_);

    attach_name(is, is.field_space, "expanded field space");

    index_space_map_[index_space_id] = std::move(is);
  }

  void
  add_adjacency(
    const adjacency_info_t& adjacency_info
  )
  {
    using namespace std;
    
    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    adjacency_t c;

    c.index_space_id = adjacency_info.index_space;
    c.from_index_space_id = adjacency_info.from_index_space;
    c.to_index_space_id = adjacency_info.to_index_space;

    auto fitr = index_space_map_.find(c.from_index_space_id);
    clog_assert(fitr != index_space_map_.end(), "invalid from index space");
    const index_space_t& fi = fitr->second;

    auto titr = index_space_map_.find(c.to_index_space_id);
    clog_assert(titr != index_space_map_.end(), "invalid to index space");
    const index_space_t& ti = titr->second;

    auto citr = adjacency_map_.find(c.index_space_id);
    clog_assert(citr == adjacency_map_.end(),
      "invalid adjacency info");
    const adjacency_t& ci = citr->second;

    c.max_conn_size = fi.total_num_entities * ti.total_num_entities;

    // Create expanded index space
    Rect<2> expanded_bounds = 
      Rect<2>(Point<2>::ZEROES(), make_point(num_colors_, c.max_conn_size));
    
    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));
    c.index_space = runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(c, c.index_space, "expanded index space");

    // Read user + FleCSI registered field spaces
    c.field_space = runtime_->create_field_space(ctx_);

    FieldAllocator allocator = 
      runtime_->create_field_allocator(ctx_, c.field_space);

    auto adjacency_offset_fid = 
      FieldID(internal_field::adjacency_offset);

    allocator.allocate_field(sizeof(size_t), adjacency_offset_fid);

    attach_name(c, c.field_space, "expanded field space");

    c.logical_region = 
      runtime_->create_logical_region(ctx_, c.index_space, c.field_space);
    attach_name(c, c.logical_region, "expanded logical region");

    clog_assert(adjacency_info.color_sizes.size() == num_colors_,
      "mismatch in color sizes");

    DomainColoring color_partitioning;
    for(size_t color = 0; color < num_colors_; ++color){
      Rect<2> subrect(make_point(color, 0), make_point(color,
        adjacency_info.color_sizes[color] - 1));
      
      color_partitioning[color] = Domain::from_rect<2>(subrect);
    }

    c.index_partition = runtime_->create_index_partition(ctx_,
      c.index_space, color_domain_, color_partitioning, true /*disjoint*/);
    attach_name(c, c.index_partition, "color partitioning");

    adjacency_map_.emplace(c.index_space_id, std::move(c));
  }

  void
  finalize(
    const indexed_coloring_info_map_t& indexed_coloring_info_map
  )
  {
    using namespace std;
    
    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    for(auto& itr : index_space_map_){
      index_space_t& is = itr.second;

      auto citr = indexed_coloring_info_map.find(is.index_space_id);
      clog_assert(citr != indexed_coloring_info_map.end(),
        "invalid index space");
      const coloring_info_map_t& coloring_info_map = citr->second;

      FieldAllocator allocator = 
        runtime_->create_field_allocator(ctx_, is.field_space);

      auto ghost_owner_pos_fid = FieldID(internal_field::ghost_owner_pos);

      allocator.allocate_field(sizeof(Point<2>), ghost_owner_pos_fid);

      using field_info_t = context_t::field_info_t;

      for(const field_info_t& fi : context.registered_fields()){
        if(fi.index_space == is.index_space_id){
          allocator.allocate_field(fi.size, fi.fid);
        }
      }

      for(auto& aitr : adjacency_map_){
        adjacency_t& a = aitr.second;

        FieldID adjacency_fid = 
          context.adjacency_fid(aitr.second.from_index_space_id,
          aitr.second.to_index_space_id);

        allocator.allocate_field(sizeof(Point<2>), adjacency_fid);
      }

      is.logical_region = runtime_->create_logical_region(ctx_, is.index_space, is.field_space);
      attach_name(is, is.logical_region, "expanded logical region");

      // Partition expanded IndexSpace color-wise & create associated PhaseBarriers
      DomainColoring color_partitioning;
      for(int color = 0; color < num_colors_; color++){
        auto citr = coloring_info_map.find(color);
        clog_assert(citr != coloring_info_map.end(), "invalid color info");
        const coloring_info_t& color_info = citr->second;

        Rect<2> subrect(
            make_point(color, 0),
            make_point(color,
              color_info.exclusive + color_info.shared + color_info.ghost - 1));
        
        color_partitioning[color] = Domain::from_rect<2>(subrect);
      }

      is.index_partition = runtime_->create_index_partition(ctx_,
        is.index_space, color_domain_, color_partitioning, true /*disjoint*/);
      attach_name(is, is.index_partition, "color partitioning");
    }
  }

  const index_space_t&
  index_space(
    size_t index_space_id
  )
  const
  {
    auto itr = index_space_map_.find(index_space_id);
    clog_assert(itr != index_space_map_.end(), "invalid index space");
    return itr->second;
  }

  const std::set<size_t>&
  index_spaces()
  const
  {
    return index_spaces_;
  }

  const Legion::Domain&
  color_domain()
  const
  {
    return color_domain_;
  }

  const std::unordered_map<size_t, adjacency_t>&
  adjacency_map()
  const
  {
    return adjacency_map_;
  }

  void
  fill_adjacency(
    size_t index_space_id,
    size_t color,
    size_t size,
    uint64_t* offsets,
    uint64_t* indices
  )
  {
    using namespace std;
    
    using namespace Legion;
    using namespace LegionRuntime;
    using namespace Arrays;

    using namespace execution;

    context_t & context = context_t::instance();

    auto itr = adjacency_map_.find(index_space_id);
    clog_assert(itr != adjacency_map_.end(), "invalid adjacency");
    adjacency_t& c = itr->second;
    index_space_t& iis = index_space_map_[c.from_index_space_id];

    IndexSpace is = h.create_index_space(0, size - 1);
    FieldSpace fs = h.create_field_space();
    FieldAllocator a = h.create_field_allocator(fs);

    auto adjacency_offset_fid = 
      FieldID(internal_field::adjacency_offset);
    
    auto adjacency_index_fid = 
      FieldID(internal_field::adjacency_index);

    a.allocate_field(sizeof(uint64_t), adjacency_offset_fid);
    a.allocate_field(sizeof(uint64_t), adjacency_index_fid);

    LogicalRegion lr = h.create_logical_region(is, fs);

    RegionRequirement rr(lr, WRITE_DISCARD, EXCLUSIVE, lr);
    rr.add_field(adjacency_offset_fid);
    rr.add_field(adjacency_index_fid);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime_->map_region(ctx_, il);
    pr.wait_until_valid();

    uint64_t* dst_offsets;
    h.get_buffer(pr, dst_offsets, adjacency_offset_fid);

    uint64_t* dst_indices;
    h.get_buffer(pr, dst_indices, adjacency_index_fid);

    std::memcpy(dst_offsets, offsets, sizeof(uint64_t) * size);
    std::memcpy(dst_indices, indices, sizeof(uint64_t) * size);

    runtime_->unmap_region(ctx_, pr);

    auto fill_adjacency_tid =
      context.task_id<__flecsi_internal_task_key(fill_adjacency_task)>();

    FieldID adjacency_fid = 
      context.adjacency_fid(c.from_index_space_id, c.to_index_space_id);

    auto p = std::make_tuple(c.from_index_space_id, c.to_index_space_id, size);    

    TaskLauncher l(fill_adjacency_tid, TaskArgument(&p, sizeof(p)));

    LogicalPartition color_conn_lp =
      runtime_->get_logical_partition(ctx_,
        c.logical_region, c.index_partition);
    
    LogicalRegion color_conn_lr =
      runtime_->get_logical_subregion_by_color(ctx_, color_conn_lp, color);

    RegionRequirement rr1(color_conn_lr, WRITE_DISCARD, SIMULTANEOUS,
      c.logical_region);
    rr1.add_field(adjacency_index_fid);
    l.add_region_requirement(rr1);

    LogicalPartition color_lp =
      runtime_->get_logical_partition(ctx_,
      iis.logical_region, iis.index_partition);
    
    LogicalRegion color_lr =
      runtime_->get_logical_subregion_by_color(ctx_, color_lp, color);

    RegionRequirement rr2(color_lr, WRITE_DISCARD, SIMULTANEOUS,
      iis.logical_region);
    rr2.add_field(adjacency_fid);
    l.add_region_requirement(rr2);

    RegionRequirement rr3(lr, READ_ONLY, SIMULTANEOUS, lr);
    rr3.add_field(adjacency_offset_fid);
    rr3.add_field(adjacency_index_fid);
    l.add_region_requirement(rr3);

    MustEpochLauncher must_epoch_launcher;
    DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, l);

    auto future = runtime_->execute_must_epoch(ctx_, must_epoch_launcher);
    future.wait_all_results();

    runtime_->destroy_index_space(ctx_, is);
    runtime_->destroy_field_space(ctx_, fs);
    runtime_->destroy_logical_region(ctx_, lr);
  }

private:
  
  Legion::Context ctx_;

  Legion::HighLevelRuntime* runtime_;

  execution::legion_helper h;

  size_t num_colors_;

  LegionRuntime::Arrays::Rect<1> color_bounds_;

  Legion::Domain color_domain_;

  std::set<size_t> index_spaces_;

  std::unordered_map<size_t, index_space_t> index_space_map_;
  
  std::unordered_map<size_t, adjacency_t> adjacency_map_;

  template<
    class T
  >
  void
  attach_name(
    const index_space_t& is,
    T& x,
    const char* label
  )
  {
    std::stringstream sstr;
    sstr << label << " " << is.index_space_id;
    runtime_->attach_name(x, sstr.str().c_str());
  }

  template<
    class T
  >
  void
  attach_name(
    const adjacency_t& c,
    T& x,
    const char* label
  )
  {
    std::stringstream sstr;
    sstr << label << " " << c.from_index_space_id << "->" <<
      c.to_index_space_id;
    runtime_->attach_name(x, sstr.str().c_str());
  }

}; // struct legion_data_t

} // namespace data

} // namespace flecsi

#endif // flecsi_legion_legion_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
