/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_generic_functor_h
#define flecsi_generic_functor_h

#include <functional>
#include <tuple>
#include <memory>
#include <utility>

/*!
 * \file generic_functor.h
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

namespace flecsi {

template<typename R> struct functor_ { virtual R operator () () = 0; };

/*!
  \class generic_functor generic_functor.h
  \brief generic_functor provides...
 */
template<typename R, typename ... Args>
class generic_functor : public functor_<R>
{
public:

  generic_functor(std::function<R (Args ...) && f, Args && ... args)
    : f_(f), args_(std::make_tuple(std::forward<Args>(args) ...)) {}

  R operator () () {
    if(std::is_same<R, void>::value) {
      eval(args_);
    }
    else {
      return eval(args_);
    } // if
  } // operator

  //! Copy constructor (disabled)
  generic_functor(const generic_functor &) = delete;

  //! Assignment operator (disabled)
  generic_functor & operator = (const generic_functor &) = delete;

  //! Destructor
   ~generic_functor() {}

private:

  template<typename ... C, size_t ... I>
  R eval(std::tuple<C ...> & t, std::index_sequence<I ...>) {
    return f_(std::get<I>(t) ...);
  } // eval

  template<typename ... C>
  R eval(std::tuple<C ...> & t) {
    return eval(t, std::make_integer_sequence<sizeof ... (C)>);
  } // eval

  std::function<R (Args ...)> f_;
  std::tuple<Args ...> args_;

}; // class generic_functor

} // namespace flecsi

#endif // flecsi_generic_functor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
