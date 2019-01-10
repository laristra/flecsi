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

#include <flecsi/utils/demangle.h>
#include <flecsi/utils/ftest.h>

inline void print_type(const char *const name)
{
   #ifdef __GNUG__
      FTEST_CAPTURE() << flecsi::utils::demangle(name) << std::endl;
   #else
      // Skip name printing; is unpredictable in this case
   #endif
}

template<class T>
inline void print_type()
{
   print_type(typeid(T).name());
}
