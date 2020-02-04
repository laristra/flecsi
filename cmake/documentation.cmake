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

if(ENABLE_SPHINX AND ENABLE_DOXYGEN)

  find_package(Git)

  if(NOT GIT_FOUND)
    message(FATAL_ERROR "Git is required for this target")
  endif()

  add_custom_target(deploy-documentation
    COMMAND
      echo "Building Sphinx" && make sphinx &&
      echo "Building Doxygen" && make doxygen &&
      echo "Updating gh-pages" &&
        ([ -e gh-pages ] ||
          ${GIT_EXECUTABLE} clone --branch gh-pages
            git@gitlab.lanl.gov:laristra/flecsi.git gh-pages) &&
      echo "Updating Sphinx pages" &&
        cp -rT doc/sphinx gh-pages &&
      echo "Updating Doxygen pages" &&
        cp -rT doc/doxygen/html gh-pages/doxygen &&
      echo "Updated gh-pages are in ${CMAKE_BINARY_DIR}/gh-pages"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

endif()
