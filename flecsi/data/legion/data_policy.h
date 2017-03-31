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

#include "legion.h"

// FIXME: This is hosed. We can't reference this here...
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

  ///
  ///
  ///
  partitioned_index_space &
  get_index_space(
    size_t is
  )
  const
  {
    auto itr = index_space_map_.find(is);
    assert(itr != index_space_map_.end() && "invalid index space");
    return const_cast<partitioned_index_space&>(itr->second);
  } // get_index_space

  ///
  ///
  ///
  void
  put_index_space(
    size_t i,
    partitioned_index_space & is
  )
  {
    index_space_map_.emplace(i, std::move(is));
  }

private:

  index_space_map index_space_map_;

}; // class legion_data_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
