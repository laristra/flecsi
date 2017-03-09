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

#include "flecsi/utils/const_string.h"

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
  using cp_t = context_policy_t;

  ///
  /// Identify the calling state of the context, i.e., this method
  /// returns the current execution level within the FleCSI model.
  ///
  enum class call_state_t : size_t {
    driver = 0,
    task,
    function,
    kernel
  }; // enum class call_state_t

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

  ///
  ///
  ///
  call_state_t
  current()
  {
    return call_state_ > 0 ? call_state_t::driver : call_state_t::task;
  } // current

  ///
  ///
  ///
  call_state_t
  entry()
  {
    return static_cast<call_state_t>(++call_state_);
  } // entry

  ///
  ///
  ///
  call_state_t
  exit()
  {
    return static_cast<call_state_t>(--call_state_);
  } // exit

  /// Copy constructor (disabled)
  context__(const context__ &) = delete;

  /// Assignment operator (disabled)
  context__ & operator = (const context__ &) = delete;

  /// Move operators
  context__(context__ &&) = default;
  context__ & operator = (context__ &&) = default;

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



private:

  /// Default constructor
  context__() : cp_t() {}

  /// Destructor
  ~context__() {}

  size_t call_state_;

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
