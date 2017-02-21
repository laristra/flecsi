/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_data_policy_h
#define flecsi_legion_data_policy_h

#include <unordered_map>
#include <map>

#include "legion.h"

namespace flecsi {
namespace data {

class legion_data_policy_t
{
public:
  struct partition
  {
    Legion::PhaseBarrier pb;
  };

  using partition_data_map = std::unordered_map<size_t, partition>;

  using partition_count_map = std::map<size_t, size_t>;

  struct partitioned_index_space
  {
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
    Legion::IndexPartition shared;
    Legion::IndexPartition exclusive;
    Legion::IndexPartition ghost;
    size_t size;
    partition_data_map pmap;
    partition_count_map pcmap;
  };

  using index_space_map = 
    std::unordered_map<size_t, partitioned_index_space>;

  partitioned_index_space& get_index_space(size_t is) const{
    auto itr = index_space_map_.find(is);
    assert(itr != index_space_map_.end() && "invalid index space");
    return const_cast<partitioned_index_space&>(itr->second);
  }

  void put_index_space(size_t i, partitioned_index_space& is){
    index_space_map_.emplace(i, std::move(is));
  }

private:
  index_space_map index_space_map_;
};

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
