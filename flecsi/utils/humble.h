/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_humble_h
#define flecsi_utils_humble_h

//!
//! \file humble.h
//!

//#include <iostream>
#include <cstdio>
#include <string>

namespace flecsi {
namespace utils {

// Macro: call here_func(), using canned values for most arguments
#define HERE(message) ::flecsi::utils::\
   here_func(__FILE__,__FUNCTION__, __LINE__,message)

// Print a diagnostic message, along with source code location
inline
void here_func(
  char const * const filename,
  char const * const fname,
  int const line,
  std::string const & s
) {
  printf("HERE %s:%s:%d %s\n",fname,filename,line,s.c_str());
}

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_humble_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
