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

#ifndef flecsi_execution_context_h
#define flecsi_execution_context_h

#include <cstddef>

///
// \file context.h
// \authors bergen
// \date Initial file creation: Oct 19, 2015
///

namespace flecsi {
namespace execution {

///
// \class context__ context.h
// \brief context__ is a dummy class that must have a specialization
//        for a specific execution policy.
///
template<class context_policy_t>
struct context__ : public context_policy_t
{
  using cp_t = context_policy_t;

  ///
  // Identify the calling state of the context, i.e., this method
  // returns the current execution level within the FleCSI model.
  ///
  enum class call_state_t : size_t {
    driver = 0,
    task,
    function,
    kernel
  }; // enum class call_state_t

  ///
  //
  ///
  static
  context__ &
  instance()
  {
    static context__ context;
    return context;
  } // instance

  ///
  //
  ///
  call_state_t
  current()
  {
    return call_state_ > 0 ? call_state_t::driver : call_state_t::task;
  } // current

  ///
  //
  ///
  call_state_t
  entry()
  {
    return static_cast<call_state_t>(++call_state_);
  } // entry

  ///
  //
  ///
  call_state_t
  exit()
  {
    return static_cast<call_state_t>(--call_state_);
  } // exit

  /// Copy constructor (disabled)
  context__(const context__ &) = delete;

  /// Assignment operator (disabled)
  context__ & operator = (const context__ &) = delete;

  /// Move operators
  context__(context__ &&) = default;
  context__ & operator = (context__ &&) = default;

private:

  /// Default constructor
  context__() : cp_t() {}

  /// Destructor
  ~context__() {}

  size_t call_state_;

}; // class context__

} // namespace execution
} // namespace flecsi

//
// This include file defines the flecsi_execution_policy_t used below.
//
#include "flecsi_runtime_context_policy.h"

namespace flecsi {
namespace execution {

using context_t = context__<flecsi_context_policy_t>;

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
