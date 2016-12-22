/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_function_h
#define flecsi_execution_function_h

#include "flecsi/utils/const_string.h"

///
// \file function.h
// \authors bergen
// \date Initial file creation: Aug 01, 2016
///

namespace flecsi {
namespace execution {

///
//
///
template<
  typename execution_policy_t
>
struct function__
{

  // FIXME: Finish Doxygen

  ///
  // \tparam R Return type.
  // \tparam A Argument type (std::tuple).
  ///
  template<
    typename R,
    typename A
  >
  static
  decltype(auto)
  register_function(
    const utils::const_string_t & key,
    std::function<R(A)> & user_function
  )
  {
    return execution_policy_t::template register_function<R, A>(key,
      user_function);
  } // register_function

  ///
  //
  ///
  template<
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_function(
    T & handle,
    As && ... args
  )
  {
    return execution_policy_t::execute_function(handle,
      std::forward<As>(args) ...);
  } // execute_function

}; // struct function__

} // namespace execution
} // namespace flecsi

//
// This include file defines the flecsi_execution_policy_t used below.
//
#include "flecsi_runtime_execution_policy.h"

namespace flecsi {
namespace execution {

using function_t = function__<flecsi_execution_policy_t>;

} // namespace function
} // namespace flecsi

#endif // flecsi_execution_function_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
