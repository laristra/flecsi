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

/*! @file */

#include <flecsi/utils/common.h>
#include <memory>
#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace flecsi {
namespace utils {

std::string
demangle(const char * const name) {
#ifdef __GNUG__
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

/*~------------------------------------------------------------------------~--*
 *~------------------------------------------------------------------------~--*/
