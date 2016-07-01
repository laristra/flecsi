/*~-------------------------------------------------------------------------~~*
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
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file static_verify.h
 *
 * \brief Utilities for performing static verification.
 *
 ******************************************************************************/
#pragma once

#ifndef FLECSI_MEMBER_CHECKER
#define FLECSI_MEMBER_CHECKER(X) template<typename T> \
struct has_member_##X{ \
  struct F{ int X; }; \
  struct D : T, F{}; \
  template<typename C, C> struct ChT; \
  template<typename C> static char (&f(ChT<int F::*, &C::X>*))[1]; \
  template<typename C> static char (&f(...))[2]; \
  static bool const value = sizeof(f<D>(0)) == 2; \
};
#endif

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
