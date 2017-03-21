/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

///
// \file legion/data_policy.h
// \authors nickm
// \date Initial file creation: Feb 24, 2017
///

#ifndef flecsi_legion_data_policy_h
#define flecsi_legion_data_policy_h

#include <unordered_map>
#include <map>

#include "legion.h"
#include "flecsi/execution/context.h"

namespace flecsi {
namespace data {

class legion_data_policy_t
{
public:

  using partitioned_index_space =
    typename flecsi::execution::context_t::partitioned_index_space;

  using index_space_map = 
    std::unordered_map<size_t, partitioned_index_space>;

  std::vector<size_t> get_keys() const{
    std::vector<size_t> keys;
    return keys;
  }

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
