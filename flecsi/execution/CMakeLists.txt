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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  context.h
)

set(execution_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
  )

  set(execution_SOURCES
    ${execution_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
  )

  set(execution_SOURCES
    ${execution_SOURCES}
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(execution_SOURCES
  ${execution_SOURCES}
  PARENT_SCOPE
)
