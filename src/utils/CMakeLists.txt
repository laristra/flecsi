#~----------------------------------------------------------------------------~#
#  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
# /@@///// /@@      /@@///// //@@   @@ /@@
# /@@      /@@      /@@       //@@ @@  /@@
# /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
# /@@////  /@@      /@@////     @@/@@  /@@
# /@@      /@@      /@@        @@ //@@ /@@
# /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
# //       //////// //////// //     // // 
# 
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#~----------------------------------------------------------------------------~#

set(utils_HEADERS
  dummy.h
  PARENT_SCOPE)

set(utils_SOURCES
  dummy.cc
  PARENT_SCOPE)

cinch_add_unit(bitfield
  SOURCES test/bitfield.cc)

cinch_add_unit(const_string
  SOURCES test/const_string.cc)

cinch_add_unit(factory
  SOURCES test/factory.cc)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
