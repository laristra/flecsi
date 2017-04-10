/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_task_hash_h
#define flecsi_execution_common_task_hash_h

#include <tuple>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"

///
/// \file
/// \date Initial file creation: Aug 16, 2016
///

namespace flecsi {
namespace execution {

///
/// This type extends std::tuple with human-readable names for our key type.
///
/// The uintptr_t is the address of the task.
/// The processor_t is the processor type of the task.
/// The bitset holds boolean values that can be interpreted by the runtime.
///
struct task_hash_key_t : public std::tuple<uintptr_t, processor_t, launch_t>
{
  using tuple_key_t = std::tuple<uintptr_t, processor_t, launch_t>;

  task_hash_key_t(const tuple_key_t & key)
    : tuple_key_t(key) {}

  ///
  /// Return the task address stored in the key.
  ///
  /// \note This is not a valid handle for executing the task and is
  ///       only used as an identifier for map lookups.
  ///
  /// \return uintptr_t & containing the address of the task.
  ///
  const
  uintptr_t &
  address() const
  {
    return std::get<0>(*this);
  } // address

  ///
  /// Return the task processor type.
  ///
  /// \return processor_t The processor type for this task.
  ///
  const
  processor_t
  processor() const
  {
    return std::get<1>(*this);
  } // processor

  ///
  /// Return the task bitset type.
  ///
  /// \return launch_t The bitset type for this task.
  ///
  const
  launch_t
  launch() const
  {
    return std::get<2>(*this);
  } // bitset

}; // struct task_hash_key_t

///
/// \struct task_hash_t execution/common/task_hash.h
///
/// The task_hash_t type provides a aggregate hashing type that combines
/// the task address and an enumerated processor type \ref processor_t
/// into a single key.  There is currently space for 256 unique processor
/// types (8 bits).
///
/// \note There is some potential for this hashing strategy to not
///       be robust. In particular, we are implicitly making the assumption
///       that the 8 most significant bits are not the only part of the
///       task address that uniquely identifies it from other tasks.
///
struct task_hash_t
{
  using key_t = task_hash_key_t::tuple_key_t;

  ///
  /// Make a key from a task address (uintptr_t) and a processor type.
  ///
  /// \return a std::pair<uintptr_t, processor_t>.
  ///
  static
  task_hash_key_t
  make_key(
    uintptr_t address,
    processor_t processor,
    launch_t launch
  )
  {
    return std::make_tuple(address, processor, launch);
  } // make_key

  ///
  /// Custom hashing function for task registry.
  ///
  /// The key is a std::tuple<uintptr_t, processor_t, launch_t>. We assume
  /// that the task address is unique even after shifting away the 12 least
  /// significant bits. There are 4 bits for the processor type, and 8 bits
  /// for the launch, i.e., there can be 16 unique processor types and
  /// 8 boolean values.
  ///
  /// \param k A hash key reference. This method is required for using
  ///          this hash type for std::map and std::unordered_map.
  ///
  std::size_t
  operator () (
    const task_hash_key_t & key
  )
  const
  {
    return (key.address() << 12) ^ (key.processor() << 8) ^
      key.launch().to_ulong();
  } // operator ()

}; // task_hash_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_common_task_hash_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
