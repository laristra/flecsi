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

#include <cinchtest.h>

#include <flecsi/utils/demangle.h>

// print_type(name)
inline void print_type(const char *const name)
{
   #ifdef __GNUG__
      CINCH_CAPTURE() << flecsi::utils::demangle(name) << std::endl;
   #else
      // Skip name printing; is unpredictable in this case
   #endif
}

// print_type<T>()
template<class T>
inline void print_type()
{
   print_type(typeid(T).name());
}

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
