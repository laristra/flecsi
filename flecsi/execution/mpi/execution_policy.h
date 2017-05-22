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

#ifndef flecsi_execution_mpi_execution_policy_h
#define flecsi_execution_mpi_execution_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 15, 2015
//----------------------------------------------------------------------------//

#include <functional>
#include <memory>
#include <type_traits>

#include <cinchlog.h>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/context.h"
//#include "flecsi/utils/const_string.h"
//#include "flecsi/utils/tuple_walker.h"
//#include "flecsi/data/data_handle.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! The mpi_execution_policy_t is the backend runtime execution policy
//! for MPI.
//!
//! @ingroup mpi-execution
//----------------------------------------------------------------------------//

struct mpi_execution_policy_t
{
  //--------------------------------------------------------------------------//
  //! The future__ type may be used for explicit synchronization of tasks.
  //!
  //! @tparam RETURN The return type of the task.
  //--------------------------------------------------------------------------//

  template<typename RETURN>
  using future__ = mpi_future__<RETURN>;

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Legion backend task registration. For documentation on this
  //! method please see task__::register_task.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE),
    size_t KEY
  >
  static
  bool
  register_task(
    task_hash_key_t key,
    std::string task_name
  )
  {
  } // register_task

  //--------------------------------------------------------------------------//
  //! MPI backend task execution. For documentation on this method,
  //! please see task__::execute_task.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    ARGS && ... args
  )
  {
    return 0;
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! MPI backend function registration. For documentation on this
  //! method, please see function__::register_function.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ... ARGS
  >
  static
  bool
  register_function(
    const utils::const_string_t & key,
    std::function<RETURN(ARGS ...)> & user_function
  )
  {
    return context_t::instance().register_function(key, user_function);
  } // register_function

  //--------------------------------------------------------------------------//
  //! MPI backend function execution. For documentation on this
  //! method, please see function__::execute_function.
  //--------------------------------------------------------------------------//

  template<
    typename FUNCTION_HANDLE,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_function(
    FUNCTION_HANDLE & handle,
    ARGS && ... args
  )
  {
    auto t = std::make_tuple(args ...);
    return handle(context_t::instance().function(handle.key()), t);
  } // execute_function

}; // struct mpi_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
