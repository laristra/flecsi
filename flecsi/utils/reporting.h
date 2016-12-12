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

#ifndef flecsi_utils_reporting_h
#define flecsi_utils_reporting_h

#include <cinchreporting.h>

/*!
 * \file reporting.h
 * \authors bergen
 * \date Initial file creation: Dec 10, 2016
 */

#if 0
#if defined(FLECSI_RUNTIME_MODEL_mpilegion)

#include <mpi.h>

namespace utils {

template<size_t R>
bool is_part() {
  int part;
  MPI_Comm_rank(MPI_COMM_WORLD, &part);
  return part == R;
} // is_part

} // namespace utils

#endif

#define flecsi_info_one(message, part)                                         \
  std::stringstream ss;                                                        \
  ss << message;                                                               \
  cinch::info_impl(ss.str(), cinch::is_part<part>)
#endif

// These macro definitions just rename the cinch reporting macros for
// use in FleCSI.

// Normally, you should wrap macro arguments in parenthesis so that
// compound arguments are expanded before they get evaluated. In this
// set of defines, we can't do this for the message string because we
// don't want it to be evaluated before it is passed into the cinch macro.
// The test argument 't' is normal and is therefore wrapped.

#define flecsi_info(s) cinch_info(s)
#define flecsi_warn(s) cinch_warn(s)
#define flecsi_error(s) cinch_error(s)
#define flecsi_assert(s) cinch_assert((t), s)

#endif // flecsi_utils_reporting_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
