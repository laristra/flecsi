/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_serial_future_policy_h
#define flecsi_execution_serial_future_policy_h

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
  typename R,
  bool is_void = std::is_void<R>::value
>
class serial_future_policy__
{
protected:

  /// Default constructor
  serial_future_policy__() {}

  /// Copy constructor
  serial_future_policy__(const serial_future_policy__ & f) {}

  /// Assignment operator
  serial_future_policy__ & operator = (const serial_future_policy__ & f) {}

  /// Destructor
  virtual ~serial_future_policy__() {}

  void
  wait()
  {
  }

}; // class serial_future_policy__

template<
  typename R
>
class serial_future_policy__<R, false>
{
protected:

  /// Constructor
  serial_future_policy__(const R return_value)
    : return_value_(return_value) {}

  /// Copy constructor
  serial_future_policy__(const serial_future_policy__ & f)
    : return_value_(f.return_value_) {}

  /// Assignment operator
  serial_future_policy__ & operator = (const serial_future_policy__ & f)
  {
    this->return_value_ = f.return_value_;
  } // operator =

  /// Destructor
  virtual ~serial_future_policy__() {}

  void
  wait()
  {
  }

  R
  get(size_t index = 0)
  {
    return return_value_;
  } // get

  const R return_value_;

}; // class serial_future_policy__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_serial_future_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
