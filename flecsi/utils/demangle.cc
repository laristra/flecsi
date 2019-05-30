/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

/*! @file */

#include <memory>

#if defined(__GNUG__)
#include <cxxabi.h>
#endif

namespace flecsi {
namespace utils {

std::string
demangle(const char * const name) {
#if defined(__GNUG__)
  int status = -4;
  std::unique_ptr<char, void (*)(void *)> res{
    abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
  if(status == 0)
    return res.get();
#endif
  // does nothing if not __GNUG__, or if abi::__cxa_demangle failed
  return name;
} // demangle

} // namespace utils
} // namespace flecsi
