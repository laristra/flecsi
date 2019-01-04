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

#include <flecsi-config.h>

#include <string>

//----------------------------------------------------------------------------//
// Set color output macros depending on whether or not FLOG_COLOR_OUTPUT
// is defined.
//----------------------------------------------------------------------------//

#ifndef FLOG_COLOR_OUTPUT

#define FLOG_COLOR_BLACK ""
#define FLOG_COLOR_DKGRAY ""
#define FLOG_COLOR_RED ""
#define FLOG_COLOR_LTRED ""
#define FLOG_COLOR_GREEN ""
#define FLOG_COLOR_LTGREEN ""
#define FLOG_COLOR_BROWN ""
#define FLOG_COLOR_YELLOW ""
#define FLOG_COLOR_BLUE ""
#define FLOG_COLOR_LTBLUE ""
#define FLOG_COLOR_PURPLE ""
#define FLOG_COLOR_LTPURPLE ""
#define FLOG_COLOR_CYAN ""
#define FLOG_COLOR_LTCYAN ""
#define FLOG_COLOR_LTGRAY ""
#define FLOG_COLOR_WHITE ""
#define FLOG_COLOR_PLAIN ""

#define FLOG_OUTPUT_BLACK(s) s
#define FLOG_OUTPUT_DKGRAY(s) s
#define FLOG_OUTPUT_RED(s) s
#define FLOG_OUTPUT_LTRED(s) s
#define FLOG_OUTPUT_GREEN(s) s
#define FLOG_OUTPUT_LTGREEN(s) s
#define FLOG_OUTPUT_BROWN(s) s
#define FLOG_OUTPUT_YELLOW(s) s
#define FLOG_OUTPUT_BLUE(s) s
#define FLOG_OUTPUT_LTBLUE(s) s
#define FLOG_OUTPUT_PURPLE(s) s
#define FLOG_OUTPUT_LTPURPLE(s) s
#define FLOG_OUTPUT_CYAN(s) s
#define FLOG_OUTPUT_LTCYAN(s) s
#define FLOG_OUTPUT_LTGRAY(s) s
#define FLOG_OUTPUT_WHITE(s) s

#else

#define FLOG_COLOR_BLACK    "\033[0;30m"
#define FLOG_COLOR_DKGRAY   "\033[1;30m"
#define FLOG_COLOR_RED      "\033[0;31m"
#define FLOG_COLOR_LTRED    "\033[1;31m"
#define FLOG_COLOR_GREEN    "\033[0;32m"
#define FLOG_COLOR_LTGREEN  "\033[1;32m"
#define FLOG_COLOR_BROWN    "\033[0;33m"
#define FLOG_COLOR_YELLOW   "\033[1;33m"
#define FLOG_COLOR_BLUE     "\033[0;34m"
#define FLOG_COLOR_LTBLUE   "\033[1;34m"
#define FLOG_COLOR_PURPLE   "\033[0;35m"
#define FLOG_COLOR_LTPURPLE "\033[1;35m"
#define FLOG_COLOR_CYAN     "\033[0;36m"
#define FLOG_COLOR_LTCYAN   "\033[1;36m"
#define FLOG_COLOR_LTGRAY   "\033[0;37m"
#define FLOG_COLOR_WHITE    "\033[1;37m"
#define FLOG_COLOR_PLAIN    "\033[0m"

#define FLOG_OUTPUT_BLACK(s) FLOG_COLOR_BLACK << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_DKGRAY(s) FLOG_COLOR_DKGRAY << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_RED(s) FLOG_COLOR_RED << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTRED(s) FLOG_COLOR_LTRED << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_GREEN(s) FLOG_COLOR_GREEN << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTGREEN(s) FLOG_COLOR_LTGREEN << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_BROWN(s) FLOG_COLOR_BROWN << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_YELLOW(s) FLOG_COLOR_YELLOW << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_BLUE(s) FLOG_COLOR_BLUE << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTBLUE(s) FLOG_COLOR_LTBLUE << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_PURPLE(s) FLOG_COLOR_PURPLE << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTPURPLE(s) FLOG_COLOR_LTPURPLE << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_CYAN(s) FLOG_COLOR_CYAN << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTCYAN(s) FLOG_COLOR_LTCYAN << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_LTGRAY(s) FLOG_COLOR_LTGRAY << s << FLOG_COLOR_PLAIN
#define FLOG_OUTPUT_WHITE(s) FLOG_COLOR_WHITE << s << FLOG_COLOR_PLAIN

#endif // FLOG_COLOR_OUTPUT

namespace flecsi {
namespace utils {
namespace flog {

/*!
  Create a timestamp.
 */

inline
std::string
timestamp(bool underscores = false)
{
  char stamp[14];
  time_t t = time(0);
  std::string format = underscores ? "%m%d_%H%M%S" : "%m%d %H:%M:%S";
  strftime(stamp, sizeof(stamp), format.c_str(), localtime(&t));
  return std::string(stamp);
} // timestamp

/*!
  Strip path from string up to last character C.

  @tparam C The character to strip.
 */

template<char C>
std::string rstrip(const char *file) {
  std::string tmp(file);
  return tmp.substr(tmp.rfind(C)+1);
} // rstrip

} // namespace flog
} // namespace utils
} // namespace flecsi
