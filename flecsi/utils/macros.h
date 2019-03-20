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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#define _FLECSI_STRINGIFY(s) #s

#define flecsi_internal_stringify(s) _FLECSI_STRINGIFY(s)

#define _FLECSI_CONCAT_IMPL(a, b) a##b
#define _FLECSI_CONCAT(a, b) _FLECSI_CONCAT_IMPL(a, b)

#define flecsi_internal_concatenate(a, b) _FLECSI_CONCAT(a, b)

#define flecsi_internal_unique_name(base) _FLECSI_CONCAT(base, __COUNTER__)
