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

# This function creates a sequential version number using a call to
# 'git describe master'

set(FLECSI_VERSION_CREATION "git describe" CACHE STRING "Set a static version")
mark_as_advanced(FLECSI_VERSION_CREATION)

if(NOT "${FLECSI_VERSION_CREATION}" STREQUAL "git describe")
  set(${PROJECT_NAME}_VERSION ${FLECSI_VERSION_CREATION})
else()

  #----------------------------------------------------------------------------#
  # Make sure that git is available
  #----------------------------------------------------------------------------#

  find_package(Git)

  if(NOT GIT_FOUND)
   message(WARNING "Git not found, using dummy version dummy-0.0.0")
    set(_version "dummy-0.0.0")
  else()

    #--------------------------------------------------------------------------#
    # Call 'git describe'
    #--------------------------------------------------------------------------#

    execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 HEAD
      OUTPUT_VARIABLE _version
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  endif()

  #----------------------------------------------------------------------------#
  # If 'git describe' failed somehow, create a dummy
  #----------------------------------------------------------------------------#

  if(NOT _version)
   message(STATUS "Git describe failed, using dummy version dummy-0.0.0")
    set(_version "dummy-0.0.0")
  endif()

  #----------------------------------------------------------------------------#
  # Set the parent scope version variable
  #----------------------------------------------------------------------------#

  set(${PROJECT_NAME}_VERSION ${_version})

  # Count the number of occurances of '.' in the version
  string(REGEX MATCHALL "\\." _matches ${_version})
  list(LENGTH _matches _matches)

  # Create list from tokens in version delimited by '.'
  string(REGEX REPLACE "\\..*" "" _major ${_version})
  string(REPLACE "." ";" version_list ${_version})

  # These must be defined because of Cinch version conventions
  list(GET version_list 0 ${PROJECT_NAME}_MAJOR)
  list(GET version_list 1 ${PROJECT_NAME}_MINOR)

  set(${PROJECT_NAME}_PATCH 0)
  if(${_matches} STREQUAL "2")
    list(GET version_list 2 ${PROJECT_NAME}_PATCH)
  endif()

endif()
