/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_future_policy_h
#define flecsi_execution_future_policy_h

///
// \file future.h
// \authors bergen
// \date Initial file creation: Oct 03, 2016
///

namespace flecsi {
namespace execution {

///
// \class serial_future_policy__ future.h
// \brief serial_future_policy__ provides...
///
template<
  typename R
>
class serial_future_policy__
{
protected:

  /// Default constructor
  serial_future_policy__(const R return_value)
    : return_value_(return_value) {}

  /// Copy constructor (disabled)
  serial_future_policy__(const serial_future_policy__ &) = delete;

  /// Assignment operator (disabled)
  serial_future_policy__ & operator = (const serial_future_policy__ &) = delete;

  /// Destructor
  virtual ~serial_future_policy__() {}

  void
  wait()
  {
  }

  R
  get(
    size_t index = 0
  )
  {
  } // get

  const R return_value_;

}; // class serial_future_policy__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_future_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
