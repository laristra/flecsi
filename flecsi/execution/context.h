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

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 19, 2015
//----------------------------------------------------------------------------//

#include <cstddef>
#include <unordered_map>

#include "cinchlog.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/partition/partition_types.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The context__ type provides a high-level runtime context interface that
//! is implemented by the given context policy.
//!
//! @tparam CONTEXT_POLICY The backend context policy.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<class CONTEXT_POLICY>
struct context__ : public CONTEXT_POLICY
{
  using index_partition_t = flecsi::dmp::index_partition_t;
  using partition_info_t = flecsi::dmp::partition_info_t;

  //---------------------------------------------------------------------------/
  //! Myer's singleton instance.
  //!
  //! @return The single instance of this type.
  //---------------------------------------------------------------------------/

  static
  context__ &
  instance()
  {
    static context__ context;
    return context;
  } // instance

  //---------------------------------------------------------------------------/
  //! Add an index partition.
  //!
  //! @param key The map key.
  //! @param partition The index partition to add.
  //---------------------------------------------------------------------------/

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

  //---------------------------------------------------------------------------/
  //! Return the index partition referenced by key.
  //!
  //! @param key The key associated with the partition to be returned.
  //---------------------------------------------------------------------------/

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

  //---------------------------------------------------------------------------/
  //! Return the partition map (convenient for iterating through all
  //! of the partitions.
  //!
  //! @return The map of index partitions.
  //---------------------------------------------------------------------------/

  const std::unordered_map<size_t, index_partition_t> &
  partitions()
  const
  {
    return partitions_;
  } // partitions

private:

  // Default constructor
  context__() : CONTEXT_POLICY() {}

  // Destructor
  ~context__() {}

  // We don't need any of these
  context__(const context__ &) = delete;
  context__ & operator = (const context__ &) = delete;
  context__(context__ &&) = delete;
  context__ & operator = (context__ &&) = delete;

  std::unordered_map<size_t, index_partition_t> partitions_;
  std::unordered_map<size_t,
    std::unordered_map<size_t, partition_info_t>> partition_info_;

}; // class context__

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_CONTEXT_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi_runtime_context_policy.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The context_t type is the high-level interface to the FleCSI runtime
//! context.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

using context_t = context__<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
