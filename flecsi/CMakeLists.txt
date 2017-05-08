#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Add public library headers
#------------------------------------------------------------------------------#

cinch_install_headers(
  FILES 
    flecsi_runtime_context_policy.h
    flecsi_runtime_data_policy.h
    flecsi_runtime_execution_policy.h
  DESTINATION include)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( concurrency )
add_subdirectory( data )
add_subdirectory( execution )
add_subdirectory( geometry )
add_subdirectory( io )
add_subdirectory( partition )
add_subdirectory( topology )
add_subdirectory( utils )

set(objects
  concurrency
  data
  execution
  io
  partition
  topology
  utils
)

set(valid_objects)
foreach( obj ${objects} )
  if ( TARGET ${obj} )
    set(valid_objects ${valid_objects} $<TARGET_OBJECTS:${obj}>)
  endif()
endforeach()

add_library(flecsi ${valid_objects})
install( TARGETS flecsi DESTINATION lib)

#~---------------------------------------------------------------------------~-#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
