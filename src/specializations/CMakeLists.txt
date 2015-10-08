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

set(specializations_HEADERS
  burton_types.h
  burton.h
  PARENT_SCOPE
)

#set(specializations_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(burton
  SOURCES test/burton.cc
  INPUT test/burton.blessed)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
