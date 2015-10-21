/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_context_h
#define flexi_context_h

/*!
 * \file context.h
 * \authors bergen
 * \date Initial file creation: Oct 19, 2015
 */

namespace flexi {

/*!
  \class context_t context.h
  \brief context_t provides...
 */
class context_t
{
public:

  enum class state_t : size_t {
    driver = 0,
    task
  }; // enum class state_t

  static context_t & instance() {
    static context_t ctx;
    return ctx;
  } // instance

  state_t current() { return state_ > 0 ? state_t::driver : state_t::task; }

  state_t entry() { return static_cast<state_t>(++state_); }
  state_t exit() { return static_cast<state_t>(--state_); }

  //! Copy constructor (disabled)
  context_t(const context_t &) = delete;

  //! Assignment operator (disabled)
  context_t & operator = (const context_t &) = delete;

private:

  context_t() : state_(static_cast<size_t>(state_t::driver)) {}
   ~context_t() {}

  size_t state_;

}; // class context_t

} // namespace flexi

#endif // flexi_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
