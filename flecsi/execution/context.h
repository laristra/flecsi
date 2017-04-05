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

#ifndef flecsi_execution_context_h
#define flecsi_execution_context_h

#include <cstddef>
#include <unordered_map>

#include "cinchlog.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/partition/index_partition.h"

///
/// \file context.h
/// \authors bergen
/// \date Initial file creation: Oct 19, 2015
///

namespace flecsi {
namespace execution {

///
/// \class context__ context.h
/// \brief context__ is a dummy class that must have a specialization
///        for a specific execution policy.
///
template<class context_policy_t>
struct context__ : public context_policy_t
{
  using index_partition_t = flecsi::dmp::index_partition_t;

  ///
  ///
  ///
  static
  context__ &
  instance()
  {
    static context__ context;
    return context;
  } // instance

  /// Copy constructor (disabled)
  context__(const context__ &) = delete;

  /// Assignment operator (disabled)
  context__ & operator = (const context__ &) = delete;

  /// Move constructor and assignment operator
  context__(context__ &&) = default;
  context__ & operator = (context__ &&) = default;

#if 0
  using partitioned_index_space = 
      typename context_policy_t::partitioned_index_space;

  //map of the all partitioned_index_spaces used in the code
  //std::map <name of the partitioned IS <entiry, index partition for entity
  std::unordered_map<utils::const_string_t,
     std::unordered_map<utils::const_string_t,
     typename context_policy_t::partitioned_index_space,
      utils::const_string_hasher_t>,
      utils::const_string_hasher_t > partitioned_index_spaces_;

  ///
  /// getting partitioned index space by name of the partition and entity
  ///
  typename context_policy_t::partitioned_index_space& get_index_space(
    utils::const_string_t part_name,
    utils::const_string_t entity
  ) const
  {
    auto itr = partitioned_index_spaces_.find(part_name);
    assert(itr != partitioned_index_spaces_.end() && "invalid index space");

    auto inner_itr=itr->second.find(entity);
    assert(inner_itr != itr->second.end() && "invalid index space");
    return  const_cast<typename context_policy_t::partitioned_index_space&>(
        inner_itr->second);
  }

  ///
  /// Adding partitioned ondex space to the context's 
  /// partitioned_index_spaces_ 
  ///
  void add_index_space(
    utils::const_string_t part_name,
    utils::const_string_t entity,
    typename context_policy_t::partitioned_index_space is
  )
  {
    std::unordered_map<utils::const_string_t,
      typename context_policy_t::partitioned_index_space,
      utils::const_string_hasher_t> map;
    map.insert({entity,is});
    partitioned_index_spaces_.emplace(part_name, std::move(map));
  }
#endif

  ///
  /// Add an index partition.
  ///
  void
  add_partition(
    size_t key,
    index_partition_t & partition
  )
  {
    if(partitions_.find(key) == partitions_.end()) {
      partitions_[key] = partition;
    } // if
  } // add_partition

  const index_partition_t &
  partition(
    size_t key
  )
  {
    if(partitions_.find(key) == partitions_.end()) {
      clog(fatal) << "invalid key " << key << std::endl;
    } // if

    return partitions_[key];
  } // partition

private:

  /// Default constructor
  context__() : context_policy_t() {}

  /// Destructor
  ~context__() {}

  std::unordered_map<size_t, index_partition_t> partitions_;

}; // class context__

} // namespace execution
} // namespace flecsi

//
// This include file defines the flecsi_execution_policy_t used below.
//
#include "flecsi_runtime_context_policy.h"

namespace flecsi {
namespace execution {

using context_t = context__<flecsi_context_policy_t>;

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
