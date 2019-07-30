#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#------------------------------------------------------------------------------#

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(CINCH_ColorReset  "${Esc}[m")
  set(CINCH_ColorBold   "${Esc}[1m")
  set(CINCH_Red         "${Esc}[31m")
  set(CINCH_Green       "${Esc}[32m")
  set(CINCH_Yellow      "${Esc}[33m")
  set(CINCH_Brown       "${Esc}[0;33m")
  set(CINCH_Blue        "${Esc}[34m")
  set(CINCH_Magenta     "${Esc}[35m")
  set(CINCH_Cyan        "${Esc}[36m")
  set(CINCH_White       "${Esc}[37m")
  set(CINCH_BoldGrey    "${Esc}[1;30m")
  set(CINCH_BoldRed     "${Esc}[1;31m")
  set(CINCH_BoldGreen   "${Esc}[1;32m")
  set(CINCH_BoldYellow  "${Esc}[1;33m")
  set(CINCH_BoldBlue    "${Esc}[1;34m")
  set(CINCH_BoldMagenta "${Esc}[1;35m")
  set(CINCH_BoldCyan    "${Esc}[1;36m")
  set(CINCH_BoldWhite   "${Esc}[1;37m")
endif()
