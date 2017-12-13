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

set(io_HEADERS
  io.h
  io_base.h
  simple_definition.h
  exodus_definition.h
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER mpi/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(io_HEADERS
    ${io_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(io_SOURCES
    ${io_SOURCES}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(simple_definition
  SOURCES test/simple_definition.cc
  INPUTS test/simple2d-8x8.msh test/simple2d-4x4.msh
)

if ( ENABLE_EXODUS )
  cinch_add_unit(exodus_definition
    SOURCES test/exodus_definition.cc
    INPUTS 
      test/exodus2d-mixed.exo
      test/exodus2d-nsided.exo
      test/exodus3d-hex.exo
      test/exodus3d-tet.exo
      test/exodus3d-nfaced.exo
    LIBRARIES ${EXODUSII_LIBRARIES}
  )
endif()


set(io_HEADERS
  ${io_HEADERS}
  io_exodus.h
  PARENT_SCOPE
)

cinch_add_unit(io
  SOURCES test/io.cc
)

set(io_SOURCES
  PARENT_SCOPE
  )
