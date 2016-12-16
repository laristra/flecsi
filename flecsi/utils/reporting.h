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

// These macro definitions just rename the cinch reporting macros for
// use in FleCSI.

// Normally, you should wrap macro arguments in parenthesis so that
// compound arguments are expanded before they get evaluated. In this
// set of defines, we can't do this for the message string because we
// don't want it to be evaluated before it is passed into the cinch macro.
// The test argument 't' is normal and is therefore wrapped.

#define flecsi_info(s) cinch_info(s)
#define flecsi_container_info(b, c, d) cinch_container_info(b, c, d)
#define flecsi_warn(s) cinch_warn(s)
#define flecsi_error(s) cinch_error(s)
#define flecsi_assert(s) cinch_assert((t), s)

// MPI reporting utilities
#if defined(HAVE_MPI)

#include <mpi.h>

namespace flecsi {
namespace utils {

///
//
///
template<
  size_t R
>
inline
bool
is_rank()
{
  int part;
  MPI_Comm_rank(MPI_COMM_WORLD, &part);
  return part == R;
} // is_rank

} // namespace utils
} // namespace flecsi

// Output modes limited to a specified MPI rank
#define flecsi_info_rank(s, r)                                                 \
  cinch_info_impl(s, flecsi::utils::is_rank<(r)>)
#define flecsi_warn_rank(s, r)                                                 \
  cinch_warn_impl(s, flecsi::utils::is_rank<(r)>)
#define flecsi_error_rank(s, r)                                                \
  cinch_error_impl(s, flecsi::utils::is_rank<(r)>)

// Print container on the specified rank
#define flecsi_container_info_rank(b, c, r, d)                                 \
  cinch_container_info_impl(b, c, d, cinch::index_bool<true>,                  \
    flecsi::utils::is_rank<(r)>)

#endif // HAVE_MPI

#endif // flecsi_utils_reporting_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
