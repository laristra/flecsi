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
  \class context_ context.h
  \brief context_ is a dummy class that must have a specialization
  	  	 for a specific execution policy.
 */
template<class context_policy_t>
class context_ : public context_policy_t
{
public:

  using cp_t = context_policy_t;

  enum class call_state_t : size_t {
    driver = 0,
    task,
    kernel
  }; // enum class call_state_t

  static context_ & instance()
  {
    static context_ ctx;
    return ctx;
  } // instance

  call_state_t current()
  {
    return call_state_ > 0 ? call_state_t::driver : call_state_t::task;
  } // current

  call_state_t entry() { return static_cast<call_state_t>(++call_state_); }
  call_state_t exit() { return static_cast<call_state_t>(--call_state_); }

  //! Copy constructor (disabled)
  context_(const context_ &) = delete;

  //! Assignment operator (disabled)
  context_ & operator = (const context_ &) = delete;

  //! Move operators
  context_(context_ &&) = default;
  context_ & operator = (context_ &&) = default;

private:

  //! Default constructor
  context_() : cp_t() {}

  //! Destructor
  ~context_() {}

private:

  size_t call_state_;

}; // class context_

} // namespace flecsi

#endif // flecsi_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
