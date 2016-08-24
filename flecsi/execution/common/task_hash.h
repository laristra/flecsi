/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_task_hash_h
#define flecsi_execution_task_hash_h

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/launch.h"

///
// \file task_hash.h
// \authors bergen
// \date Initial file creation: Aug 16, 2016
///

namespace flecsi {
namespace execution {

///
// \struct task_hash_t execution/common/task_hash.h
//
// The task_hash_t type provides a aggregate hashing type that combines
// the task address and an enumerated processor type \ref processor_t
// into a single key.  There is currently space for 256 unique processor
// types (8 bits).
//
// \note There is some potential for this hashing strategy to not
//       be robust. In particular, we are implicitly making the assumption
//       that the 8 most significant bits are not the only part of the
//       task address that uniquely identifies it from other tasks. This
//       also assumes that function addresses are at least half-byte aligned.
///
struct task_hash_t
{
  using key_t = std::tuple<uintptr_t, processor_t, launch_t>;

  ///
  // Make a key from a task address (uintptr_t) and a processor type.
  //
  // \return a std::pair<uintptr_t, processor_t>.
  ///
  static
  key_t
  make_key(
    uintptr_t address,
    processor_t processor,
    launch_t launch_type
  )
  {
    return std::make_tuple(address, processor, launch_type);
  } // make_key

  ///
  // Custom hashing function for task registry.
  ///
  std::size_t
  operator () (
    const key_t & k
  )
  const
  {
    // FIXME: We can use a better hash strategy
    return (std::get<0>(k) << 8) ^ (std::get<1>(k) << 4) ^ std::get<2>(k);
  } // operator ()

}; // task_hash_t

using task_hash_key_t = task_hash_t::key_t;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_task_hash_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
