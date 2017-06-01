/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_function_h
#define flecsi_execution_function_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include "flecsi/utils/const_string.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The function__ type provides a high-level function interface that is
//! implemented by the given execution policy.
//!
//! @tparam EXECUTION_POLICY The backend execution policy.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<
  typename EXECUTION_POLICY
>
struct function__
{

  //--------------------------------------------------------------------------//
  //! Register a user function with the FleCSI runtime.
  //!
  //! @todo: This interface needs to be updated to mirror the task
  //!        registration model.
  //!
  //! @tparam RETURN    The return type of the user function.
  //! @tparam ARG_TUPLE A std::tuple of the user function arguments.
  //!
  //! @param key           The function hash key.
  //! @param user_function The user function.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE),
    size_t KEY
  >
  static
  decltype(auto)
  register_function()
  {
    return EXECUTION_POLICY::template register_function<
      RETURN, ARG_TUPLE, FUNCTION, KEY>();
  } // register_function

  //--------------------------------------------------------------------------//
  //! Execute a registered function.
  //!
  //! @todo: This interface needs to be updated to mirror the task
  //!        registration model.
  //!
  //! @tparam FUNCTION_HANDLE The function handle type.
  //! @tparam ARGS            A variadic pack of the user function arguments.
  //!
  //! @param key           The function hash key.
  //! @param user_function The user function.
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
    return EXECUTION_POLICY::template execute_function(handle,
      std::forward<ARGS>(args) ...);
  } // execute_function

}; // struct function__

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_EXECUTION_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi_runtime_execution_policy.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! Use the execution policy to define the function type.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

using function_t = function__<FLECSI_RUNTIME_EXECUTION_POLICY>;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_function_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
