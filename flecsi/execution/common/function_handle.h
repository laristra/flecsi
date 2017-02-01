/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_function_handle_h
#define flecsi_execution_common_function_handle_h

#include <functional>

#include "flecsi/utils/tuple_function.h"

///
// \file function_handle.h
// \authors bergen
// \date Initial file creation: Aug 04, 2016
///

namespace flecsi {
namespace execution {

///
// \class function_handle__ function_handle.h
// \brief function_handle__ provides...
//
// \tparam R Return value type.
// \tparam A Argument type (std::tuple).
///
template<
  typename R,
  typename A
>
struct function_handle__
{

  constexpr function_handle__(const size_t key_)
    : key(key_) {}

  ///
  //
  ///
  template< typename T >
  R
  operator () (
    std::function<void(void)> * user_function,
    T && args
  )
  {
    auto & kr =
      *(reinterpret_cast<std::function<R(A)> *>(user_function));
    return kr( std::forward<T>(args));
  } // operator ()

  size_t key;

}; // class function_handle__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_common_function_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
