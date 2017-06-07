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

///
/// \file legion/legion_data.h
/// \date Initial file creation: Jun 7, 2017
///

namespace flecsi{

namespace data{

class legion_data_t{
public:
  using coloring_info_t = coloring::coloring_info_t;

  using coloring_info_map_t = std::unordered_map<size_t, coloring_info_t>;

  using indexed_coloring_info_map_t = 
    std::unordered_map<size_t, std::unordered_map<size_t, coloring_info_t>>;

  struct index_space_info_t{
    size_t index_space_id;
    Legion::IndexSpace is;
    Legion::FieldSpace fs;
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
  };

  legion_data_t(
    Legion::Context ctx,
    Legion::Runtime* runtime,
    size_t num_colors
  )
  : ctx_(ctx),
    runtime_(runtime),
    num_colors_(num_colors),
    color_bounds_(0, num_colors_ - 1),
    color_domain_(Legion::Domain::from_rect<1>(color_bounds_))
  {
  }

  void
  init_from_coloring_info_map(
    const indexed_coloring_info_map_t& indexed_coloring_info_map
  )
  {
    for(auto idx_space : indexed_coloring_info_map){
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

    index_space_info_t is;
    is.index_space_id = index_space_id;

    // Create expanded IndexSpace
    index_spaces_.insert(index_space_id);

    // Determine max size of a color partition
    size_t total_num_entities = 0;
    for(auto color_idx : coloring_info_map){
      clog(trace) << "index: " << index_space_id << " color: " << 
        color_idx.first << " " << color_idx.second << std::endl;
      
      total_num_entities = std::max(total_num_entities,
        color_idx.second.exclusive + color_idx.second.shared + 
        color_idx.second.ghost);
    } // for color_idx
    
    clog(trace) << "total_num_entities " << total_num_entities << std::endl;

    // Create expanded index space
    Rect<2> expanded_bounds = 
      Rect<2>(Point<2>::ZEROES(), make_point(num_colors_, total_num_entities));
    
    Domain expanded_dom(Domain::from_rect<2>(expanded_bounds));
    
    is.is = runtime_->create_index_space(ctx_, expanded_dom);
    attach_name(is, is.is, "expanded index space");

    // Read user + FleCSI registered field spaces
    is.fs = runtime_->create_field_space(ctx_);

    FieldAllocator allocator = 
      runtime_->create_field_allocator(ctx_, is.fs);

    auto ghost_owner_pos_fid = 
      LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

    allocator.allocate_field(sizeof(Point<2>), ghost_owner_pos_fid);

    using field_info_t = context_t::field_info_t;

    for(const field_info_t& fi : context.registered_fields()){
      if(fi.index_space == index_space_id){
        allocator.allocate_field(fi.size, fi.fid);
      }
    }

    attach_name(is, is.fs, "expanded field space");

    is.lr = runtime_->create_logical_region(ctx_, is.is, is.fs);
    attach_name(is, is.lr, "expanded logical region");

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

    is.ip = runtime_->create_index_partition(ctx_,
      is.is, color_domain_, color_partitioning, true /*disjoint*/);
    attach_name(is, is.ip, "color partitioning");

    index_space_map_[index_space_id] = std::move(is);
  }

  const index_space_info_t&
  index_space_info(size_t index_space)
  const
  {
    auto itr = index_space_map_.find(index_space);
    clog_assert(itr != index_space_map_.end(), "invalid index space");
    return itr->second;
  }

private:
  
  Legion::Context ctx_;

  Legion::HighLevelRuntime* runtime_;

  size_t num_colors_;

  LegionRuntime::Arrays::Rect<1> color_bounds_;

  Legion::Domain color_domain_;

  std::set<size_t> index_spaces_;

  std::unordered_map<size_t, index_space_info_t> index_space_map_;

  template<
    class T
  >
  void
  attach_name(const index_space_info_t& is, T&& x, const char* label)
  {
    std::stringstream sstr;
    sstr << label << " " << is.index_space_id;
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
