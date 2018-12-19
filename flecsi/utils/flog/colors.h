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

//----------------------------------------------------------------------------//
// Set color output macros depending on whether or not FLOG_COLOR_OUTPUT
// is defined.
//----------------------------------------------------------------------------//

#ifndef FLOG_COLOR_OUTPUT

#define COLOR_BLACK ""
#define COLOR_DKGRAY ""
#define COLOR_RED ""
#define COLOR_LTRED ""
#define COLOR_GREEN ""
#define COLOR_LTGREEN ""
#define COLOR_BROWN ""
#define COLOR_YELLOW ""
#define COLOR_BLUE ""
#define COLOR_LTBLUE ""
#define COLOR_PURPLE ""
#define COLOR_LTPURPLE ""
#define COLOR_CYAN ""
#define COLOR_LTCYAN ""
#define COLOR_LTGRAY ""
#define COLOR_WHITE ""
#define COLOR_PLAIN ""

#define OUTPUT_BLACK(s) s
#define OUTPUT_DKGRAY(s) s
#define OUTPUT_RED(s) s
#define OUTPUT_LTRED(s) s
#define OUTPUT_GREEN(s) s
#define OUTPUT_LTGREEN(s) s
#define OUTPUT_BROWN(s) s
#define OUTPUT_YELLOW(s) s
#define OUTPUT_BLUE(s) s
#define OUTPUT_LTBLUE(s) s
#define OUTPUT_PURPLE(s) s
#define OUTPUT_LTPURPLE(s) s
#define OUTPUT_CYAN(s) s
#define OUTPUT_LTCYAN(s) s
#define OUTPUT_LTGRAY(s) s
#define OUTPUT_WHITE(s) s

#else

#define COLOR_BLACK    "\033[0;30m"
#define COLOR_DKGRAY   "\033[1;30m"
#define COLOR_RED      "\033[0;31m"
#define COLOR_LTRED    "\033[1;31m"
#define COLOR_GREEN    "\033[0;32m"
#define COLOR_LTGREEN  "\033[1;32m"
#define COLOR_BROWN    "\033[0;33m"
#define COLOR_YELLOW   "\033[1;33m"
#define COLOR_BLUE     "\033[0;34m"
#define COLOR_LTBLUE   "\033[1;34m"
#define COLOR_PURPLE   "\033[0;35m"
#define COLOR_LTPURPLE "\033[1;35m"
#define COLOR_CYAN     "\033[0;36m"
#define COLOR_LTCYAN   "\033[1;36m"
#define COLOR_LTGRAY   "\033[0;37m"
#define COLOR_WHITE    "\033[1;37m"
#define COLOR_PLAIN    "\033[0m"

#define OUTPUT_BLACK(s) COLOR_BLACK << s << COLOR_PLAIN
#define OUTPUT_DKGRAY(s) COLOR_DKGRAY << s << COLOR_PLAIN
#define OUTPUT_RED(s) COLOR_RED << s << COLOR_PLAIN
#define OUTPUT_LTRED(s) COLOR_LTRED << s << COLOR_PLAIN
#define OUTPUT_GREEN(s) COLOR_GREEN << s << COLOR_PLAIN
#define OUTPUT_LTGREEN(s) COLOR_LTGREEN << s << COLOR_PLAIN
#define OUTPUT_BROWN(s) COLOR_BROWN << s << COLOR_PLAIN
#define OUTPUT_YELLOW(s) COLOR_YELLOW << s << COLOR_PLAIN
#define OUTPUT_BLUE(s) COLOR_BLUE << s << COLOR_PLAIN
#define OUTPUT_LTBLUE(s) COLOR_LTBLUE << s << COLOR_PLAIN
#define OUTPUT_PURPLE(s) COLOR_PURPLE << s << COLOR_PLAIN
#define OUTPUT_LTPURPLE(s) COLOR_LTPURPLE << s << COLOR_PLAIN
#define OUTPUT_CYAN(s) COLOR_CYAN << s << COLOR_PLAIN
#define OUTPUT_LTCYAN(s) COLOR_LTCYAN << s << COLOR_PLAIN
#define OUTPUT_LTGRAY(s) COLOR_LTGRAY << s << COLOR_PLAIN
#define OUTPUT_WHITE(s) COLOR_WHITE << s << COLOR_PLAIN

#endif // FLOG_COLOR_OUTPUT
