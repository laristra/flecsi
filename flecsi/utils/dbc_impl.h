/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */


#ifndef IM_OK_TO_INCLUDE_DBC_IMPL
#warning                                                                       \
    "You almost certainly do not want to include this dbc_impl.h--use dbc.h!"
#endif

#include <functional> // std::function
#include <string>

/* Implementations for the DBC system. Do not include this directly. */

namespace flecsi {
namespace dbc {

using action_t =
    std::function<void(std::string const &, const char *, const char *, int)>;

/**\brief Constructs error/exception message */
std::string build_message(
    std::string const & cond,
    const char * file_name,
    const char * func_name,
    int line);

/**\brief Stream used for error reporting, defaults to std::cerr. */
extern std::ostream * p_str;

/**\brief Core assertion function. */
bool assertf(
    bool cond,
    std::string const & cond_str,
    const char * file_name,
    const char * func_name,
    int line,
    action_t &);

/**\brief Core assertion function, with some hope of not constructing the
 * error string every single time?
 */
template<class error_msg_generator>
bool
assertf_l(
    bool cond,
    error_msg_generator & gen,
    const char * file_name,
    const char * func_name,
    int line,
    action_t & action) {
  if (!cond) {
    std::string errstr = gen();
    action(errstr, file_name, func_name, line);
  }
  return cond;
}

} // namespace dbc
} // namespace flecsi

// End of file
