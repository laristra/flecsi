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
  typename policy_t
>
class future_u__
{
public:

  /// Default constructor
  template<typename ... As>
  future_u__(As && ... args)
    : policy_t(std::forward<As>(args ...))
  {}

  /// Copy constructor (disabled)
  future_u__(const future_u__ &) = delete;

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
    policy_t::wait();
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
    return policy_t::get(index);
  } // get

}; // class future_u__

} // namespace execution
} // namespace flecsi

//
// This include file defines the flecsi_future_policy_t used below.
//
#include "flecsi_runtime_execution_policy.h"

namespace flecsi {
namespace execution {

template<typename R>
using future__ = future_u__<R, flecsi_future_policy_t>;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_future_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
