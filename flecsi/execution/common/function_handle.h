/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_function_handle_h
#define flecsi_execution_common_function_handle_h

#include <functional>

#include "flecsi/utils/tuple_function.h"

//----------------------------------------------------------------------------//
//! @file function_handle.h
//! @date Initial file creation: Aug 04, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//!
//! \tparam RETURN Return value type.
//! \tparam ARG_TUPLE Argument type (std::tuple).
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  typename ARG_TUPLE
>
struct function_handle__
{
  using return_t = RETURN;
  using arg_tuple_t = ARG_TUPLE;

  ///
  /// Constructor.
  ///
  /// \param key A hash key identifier for the function.
  /// 
  constexpr function_handle__(const size_t key)
    : key_(key) {}

  ///
  /// Execute the function.
  ///
  RETURN
  operator () (
    void * function,
    ARG_TUPLE && args
  )
  {
    auto user_function = (reinterpret_cast<RETURN(*)(ARG_TUPLE)>(function));
    return user_function(std::forward<ARG_TUPLE>(args));
  } // operator ()

  ///
  /// Return the identifier key.
  ///
  size_t
  key()
  const
  {
    return key_;
  } // key

private:

  size_t key_;

}; // class function_handle__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_common_function_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
