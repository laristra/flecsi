/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

/// \file


#ifndef flecsi_utils_humble_h
#define flecsi_utils_humble_h

#include <iostream>
#include <string>

namespace flecsi {
namespace utils {

#define HERE(message) here_func(__FILE__,__FUNCTION__, __LINE__,message)

/** Print a diagnostic message, along with source code location */
inline
void here_func(const char *filename, const char * fname, int line,
    std::string const & s){
  printf("HERE %s:%s:%d %s\n",fname,filename,line,s.c_str());
}


} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_humble_h

