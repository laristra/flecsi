/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_tuple_wrapper_h
#define flecsi_utils_tuple_wrapper_h

#include <tuple>

/*!
 * \file
 * \date Initial file creation: Jul 28, 2016
 */

namespace flecsi {
namespace utils {

struct generic_tuple_t {};

/*!
  \class legion_argument_wrapper legion_argument_wrapper.h
  \brief legion_argument_wrapper provides...
 */
template<typename ... Args>
struct tuple_wrapper_ : generic_tuple_t {

  using tuple_t = std::tuple<Args ...>;

  tuple_wrapper_(Args ... args) : t_(std::make_tuple(args ...)) {}

  template<size_t I>
  decltype(auto) get() { return std::get<I>(t_); }
  
private:

  tuple_t t_;

}; // class tuple_wrapper_

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_tuple_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
