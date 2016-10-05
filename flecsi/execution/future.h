/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_future_h
#define flecsi_execution_future_h

///
// \file future.h
// \authors bergen
// \date Initial file creation: Oct 03, 2016
///

namespace flecsi {
namespace execution {

///
// \class future_u__ future.h
// \brief future_u__ provides...
///
template<
  typename R,
  template <typename S, bool is_void> class policy__
>
class future_u__ : policy__<R, std::is_void<R>::value>
{
public:

  /// Constructor for non-void return type
  template<typename S = R>
  future_u__(typename std::enable_if<!std::is_void<S>::value, S>::type && arg)
    : policy__<S, std::is_void<S>::value>(std::forward<S>(arg))
  {
    static_assert(std::is_trivially_copyable<S>::value,
      "non-trivial types not supported");
  } // future_u__

  /// Constructor for void return type
  future_u__() : policy__<R, std::is_void<R>::value>() {}

  /// Copy constructor
  future_u__(const future_u__ & f)
    : policy__<R, std::is_void<R>::value>(f) {}

  /// Assignment operator (disabled)
  future_u__ & operator = (const future_u__ &) = delete;

  /// Destructor
   ~future_u__() {}

  ///
  // Wait on the associated task to complete.
  ///
  void
  wait()
  {
    policy__<R, std::is_void<R>::value>::wait();
  } // wait

  ///
  // Get the return value from the associated task.
  //
  // \param index The index of the sub-task from which to get the
  //              return value. This parameter defaults to zero, so
  //              that single task instances do not have to specify
  //              an identifier.
  ///
  R
  get(
    size_t index = 0
  )
  {
    return policy__<R, std::is_void<R>::value>::get(index);
  } // get

}; // class future_u__

} // namespace execution
} // namespace flecsi

//
// This include file defines the flecsi_future_policy_t used below.
//
#include "flecsi_runtime_context_policy.h"

namespace flecsi {
namespace execution {

template<typename R>
using future__ = future_u__<R, flecsi_future_policy__>;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_future_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
