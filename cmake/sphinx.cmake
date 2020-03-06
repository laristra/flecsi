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

include(copy_directory)

option(ENABLE_SPHINX "Enable Sphinx documentation" OFF)

if(ENABLE_SPHINX)

    find_package(Sphinx REQUIRED)

    #--------------------------------------------------------------------------#
    # This variable is used in the Sphinx configuration file.  It
    # will be used in the configure_file call below.
    #--------------------------------------------------------------------------#

    set(${PROJECT_NAME}_SPHINX_TARGET sphinx)

    #--------------------------------------------------------------------------#
    # Create directories for intermediate files
    #--------------------------------------------------------------------------#

    if(NOT EXISTS ${CMAKE_BINARY_DIR}/doc/.sphinx)
      file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/.sphinx)
    endif()

    if(NOT EXISTS ${CMAKE_BINARY_DIR}/doc/.sphinx/_static)
      file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/.sphinx/_static)
    endif()

    copy_directory(${CMAKE_CURRENT_SOURCE_DIR}/sphinx/_static
        doc/.sphinx/_static)

      if(NOT EXISTS ${CMAKE_BINARY_DIR}/doc/.sphinx/_templates)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/.sphinx/_templates)
    endif()

    copy_directory(${CMAKE_CURRENT_SOURCE_DIR}/sphinx/_templates
        doc/.sphinx/_templates)

    #--------------------------------------------------------------------------#
    # Generate the Sphinx config file
    #--------------------------------------------------------------------------#

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sphinx/conf.py.in
      ${CMAKE_BINARY_DIR}/doc/.sphinx/conf.py)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sphinx/index.rst
      DESTINATION ${CMAKE_BINARY_DIR}/doc/.sphinx)

    #--------------------------------------------------------------------------#
    # Add the Sphinx target
    #--------------------------------------------------------------------------#

    add_custom_target(sphinx
        COMMAND ${SPHINX_EXECUTABLE} -Q -b html
        -c ${CMAKE_BINARY_DIR}/doc/.sphinx
            ${CMAKE_CURRENT_SOURCE_DIR}/sphinx
            ${CMAKE_BINARY_DIR}/doc/sphinx
    )

    add_custom_target(sphinx-man
        COMMAND ${SPHINX_EXECUTABLE} -q -b man
        -c ${CMAKE_BINARY_DIR}/doc/.sphinx
            ${CMAKE_CURRENT_SOURCE_DIR}/sphinx
            ${CMAKE_BINARY_DIR}/doc/sphinx
    )

    #--------------------------------------------------------------------------#
    # Add install target
    #--------------------------------------------------------------------------#

    add_custom_target(install-sphinx
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/doc/sphinx
        $ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})

    endif()
