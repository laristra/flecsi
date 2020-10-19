#------------------------------------------------------------------------------#
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

include_directories(${PROJECT_SOURCE_DIR})

#------------------------------------------------------------------------------#
# Add an executable.
#------------------------------------------------------------------------------#

add_executable(standalone
  standalone.cc
  advance.hh
  analyze.hh
  data.hh
  finalize.hh
  initialize.hh
  ../specialization/control.hh
  ../specialization/types.hh
)

#------------------------------------------------------------------------------#
# Add the FleCSI libraries and dependencies.
#------------------------------------------------------------------------------#

target_link_libraries(standalone
  ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES}
)
