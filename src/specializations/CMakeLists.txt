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
  unstruct_types.h
  unstruct.h
  PARENT_SCOPE
)
if(ENABLE_IO)
  list(APPEND specializations_HEADERS burton_io_exodus.h)
endif(ENABLE_IO)


#set(specializations_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(burton
  SOURCES test/burton.cc
  INPUTS test/burton.blessed
)

if(ENABLE_IO)
  cinch_add_unit(burton_io
    SOURCES test/burton_io.cc
    LIBRARIES ${IO_LIBRARIES}
  )
endif(ENABLE_IO)

#cinch_add_unit(unstruct
#  SOURCES test/unstruct.cc
#  INPUTS test/unstruct.blessed)


#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
