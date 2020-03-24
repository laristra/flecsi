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

include(CMakeDependentOption)

option(ENABLE_DOCUMENTATION "Enable documentation" OFF)
mark_as_advanced(ENABLE_DOCUMENTATION)

cmake_dependent_option(ENABLE_DOXYGEN "Enable Doxygen documentation"
  ON "ENABLE_DOCUMENTATION" OFF)
mark_as_advanced(ENABLE_DOXYGEN)

cmake_dependent_option(ENABLE_DOXYGEN_WARN "Enable Doxygen warnings"
  OFF "ENABLE_DOCUMENTATION" OFF)
mark_as_advanced(ENABLE_DOXYGEN_WARN)

cmake_dependent_option(ENABLE_SPHINX "Enable Sphinx documentation"
  ON "ENABLE_DOCUMENTATION" OFF)
mark_as_advanced(ENABLE_SPHINX)

if(ENABLE_DOCUMENTATION)

  if(ENABLE_DOXYGEN)
    find_package(Doxygen REQUIRED)

    # This is used by configure_file
    set(DOXYGEN_WARN NO)
    if(ENABLE_DOXYGEN_WARN)
      set(DOXYGEN_WARN YES)
    endif()

    # This is used by configure_file
    set(${PROJECT_NAME}_DOXYGEN_TARGET doxygen)

    configure_file(${PROJECT_SOURCE_DIR}/config/doxygen.conf.in
      ${CMAKE_BINARY_DIR}/doc/.doxygen/doxygen.conf)

    add_custom_target(doxygen
        ${DOXYGEN} ${CMAKE_BINARY_DIR}/doc/.doxygen/doxygen.conf
        DEPENDS ${PROJECT_SOURCE_DIR}/config/doxygen.conf.in)
  endif()

  if(ENABLE_SPHINX)
    find_package(Sphinx REQUIRED)

    file(COPY ${CMAKE_SOURCE_DIR}/config/sphinx/_static
      DESTINATION ${CMAKE_BINARY_DIR}/doc/.sphinx)

    file(COPY ${CMAKE_SOURCE_DIR}/config/sphinx/_templates
      DESTINATION ${CMAKE_BINARY_DIR}/doc/.sphinx)

    configure_file(${CMAKE_SOURCE_DIR}/config/sphinx/conf.py.in
      ${CMAKE_BINARY_DIR}/doc/.sphinx/conf.py)

    add_custom_target(sphinx
      COMMAND ${SPHINX_EXECUTABLE} -Q -b html -c
        ${CMAKE_BINARY_DIR}/doc/.sphinx
        ${CMAKE_SOURCE_DIR}/config/sphinx
        ${CMAKE_BINARY_DIR}/doc/sphinx
    )
  endif()

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

endif()
