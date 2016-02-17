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

#ifndef flecsi_common_h
#define flecsi_common_h

#include <cstdint>

#include "id.h"

/*!
 * \file common.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

#ifndef FLECSI_ID_PBITS
#define FLECSI_ID_PBITS 20
#endif

#ifndef FLECSI_ID_EBITS
#define FLECSI_ID_EBITS 40
#endif

namespace flecsi
{
using id_t = id_<FLECSI_ID_PBITS, FLECSI_ID_EBITS>;

//! P.O.D.
template <typename T>
T square(const T & a)
{
  return a * a;
}

} // namespace flecsi

#define _UTIL_STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) _UTIL_STRINGIFY(s)

#endif // flecsi_common_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
