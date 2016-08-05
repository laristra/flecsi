/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_function_handle_h
#define flecsi_function_handle_h

#include <functional>

#include "flecsi/utils/tuple_function.h"

/*!
 * \file function_handle.h
 * \authors bergen
 * \date Initial file creation: Aug 04, 2016
 */

namespace flecsi {
namespace execution {

/*!
  \class function_handle__ function_handle.h
  \brief function_handle__ provides...
 */
template<typename R, typename ... As>
struct function_handle__
{
  using argument_t = std::tuple<As ...>;

  constexpr function_handle__(const size_t key_)
    : key(key_) {}

  R operator () (std::function<void(void)> * user_function, argument_t & args)
  {
    std::function<R(As ...)> & kr =
      *(reinterpret_cast<std::function<R(As ...)> *>(user_function));
    return tuple_function(kr, args);
  } // operator ()

  size_t key;
}; // class function_handle__

} // namespace execution
} // namespace flecsi

#endif // flecsi_function_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
