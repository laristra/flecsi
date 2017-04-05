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
  /// Myer's singleton instance.
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

  ///
  /// Add an index partition.
  ///
  /// \param key The map key.
  /// \param partition The partition to add.
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

  ///
  /// Return the partition referenced by key.
  ///
  /// \param key The key associated with the partition to be returned.
  ///
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

  ///
  /// Return the partition map (convenient for iterating through all
  /// of the partitions.
  ///
  const std::unordered_map<size_t, index_partition_t> &
  partitions()
  const
  {
    return partitions_;
  } // partitions

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
