#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

set(specializations_HEADERS
  burton/burton_entity_types.h
  burton/burton.h
  burton/burton_io_exodus.h
  burton/burton_mesh.h
  burton/burton_mesh_traits.h
  burton/burton_types.h
  PARENT_SCOPE
)
if(ENABLE_IO)
  list(APPEND specializations_HEADERS burton_io_exodus.h)
endif(ENABLE_IO)


set(specializations_SOURCES
  burton/burton_entity_types.cc
  PARENT_SCOPE
)

cinch_add_unit(burton
  SOURCES burton/test/burton.cc
  INPUTS burton/test/burton.blessed
  LIBRARIES flecsi
)

cinch_add_unit(copy
  SOURCES burton/test/copy.cc
  LIBRARIES flecsi
)

cinch_add_unit(connectivity
  SOURCES burton/test/connectivity.cc
  LIBRARIES flecsi
)

if(LAPACKE_FOUND AND ENABLE_IO)
  cinch_add_unit(burton_gradients
    SOURCES burton/test/burton_gradients.cc
    LIBRARIES flecsi ${LAPACK_LIBRARIES} ${IO_LIBRARIES}
  )
endif(LAPACKE_FOUND AND ENABLE_IO)

if(ENABLE_IO)
  cinch_add_unit(burton_io
    SOURCES burton/test/burton_io.cc
    LIBRARIES ${IO_LIBRARIES} flecsi
  )
endif(ENABLE_IO)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
