// assert.cc
// T. M. Kelley
// May 02, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved

#define IM_OK_TO_INCLUDE_DBC_IMPL
#include "dbc_impl.h"
#undef IM_OK_TO_INCLUDE_DBC_IMPL
#include <iostream>
#include <sstream>

namespace flecsi{

namespace dbc{

std::ostream *p_str = &std::cerr;

std::string build_message(std::string const & cond,const char * file_name,
  const char * func_name, int line){
  std::stringstream s;
  s << file_name << ":" << line << ":" << func_name << " assertion '"
    << cond << "' failed." << std::endl;
  return s.str();
} // build_message

bool assertf(bool cond, std::string const & cond_str, const char * file_name,
  const char * func_name, int line, action_t &action){
  if(!cond){
    action(cond_str,file_name,func_name,line);
  }
  return cond;
} // assertf

} // dbc::

} // flecsi::


// End of file
