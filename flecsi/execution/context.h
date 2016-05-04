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

#ifndef flecsi_context_h
#define flecsi_context_h

#include <cstddef>

/*!
 * \file context.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flecsi
{
/*!
  \class context_t context.h
  \brief context_t is a dummy class that must have a specialization
  	  	 for a specific execution policy.
 */
template<class execution_policy>
class context_t : public execution_policy::context_ep
{
 public:
  enum class call_state_t : size_t {
    driver = 0,
    task
  }; // enum class call_state_t

  static context_t & instance()
  {
    static context_t ctx;
    return ctx;
  } // instance

  call_state_t current()
  {
    return call_state_ > 0 ? call_state_t::driver : call_state_t::task;
  } // current

  call_state_t entry() { return static_cast<call_state_t>(++call_state_); }
  call_state_t exit() { return static_cast<call_state_t>(--call_state_); }
  //! Copy constructor (disabled)
//  context_t(const context_t &) = delete;

  //! Assignment operator (disabled)
//  context_t & operator=(const context_t &) = delete;
  context_t() : call_state_(static_cast<size_t>(call_state_t::driver)) {}


  template<class... args>
  context_t(size_t call_state, args... a):call_state_(call_state),execution_policy::context_ep(a...){}

 private:


//  ~context_t() {}
  size_t call_state_;



}; // class context_t

} // namespace flecsi

#endif // flecsi_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
