// humble.h
// T. M. Kelley
// Aug 16, 2016
// (c) Copyright 2016 LANSLLC, all rights reserved


#ifndef HUMBLE_H
#define HUMBLE_H

#include <iostream>
#include <string>

// namespace flecsi{
//
// namespace utils {

#define HERE(message) here_func(__FILE__,__FUNCTION__, __LINE__,message)

/** Print a diagnostic message, along with source code location */
inline
void here_func(const char *filename, const char * fname, int line,
    std::string const & s){
  printf("HERE %s:%s:%d %s\n",fname,filename,line,s.c_str());
}


// } // utils::
//
// } // flecsi::

#endif // include guard


// End of file
