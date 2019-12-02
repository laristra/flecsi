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

include(subdirlist)

function(add_library_target target directory)

  #----------------------------------------------------------------------------#
  # Setup argument options
  #----------------------------------------------------------------------------#

  set(options)
  set(one_value_args EXPORT_TARGET)
  set(multi_value_args)

  cmake_parse_arguments(lib "${options}" "${one_value_args}"
    "${multi_value_args}" ${ARGN})

  #----------------------------------------------------------------------------#
  # Add target to list
  #----------------------------------------------------------------------------#

  message(STATUS
    "Adding library target ${target} with source directory ${directory}")

  #----------------------------------------------------------------------------#
  # Add public headers
  #----------------------------------------------------------------------------#

  set(_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${directory})

  if(EXISTS ${_SOURCE_DIR}/library.cmake)
    include(${_SOURCE_DIR}/library.cmake)
  endif()

  foreach(_HEADER ${${directory}_HEADERS})
    if(NOT EXISTS ${_SOURCE_DIR}/${_HEADER})
      message(FATAL_ERROR "Header '${_HEADER}' from ${directory}_HEADERS does not exist.")
    endif()
    list(APPEND HEADERS
      ${_HEADER})
    list(APPEND GLOBAL_HEADERS ${_SOURCE_DIR}/${_HEADER})
  endforeach()

  foreach(_SOURCE ${${directory}_SOURCES})
    list(APPEND SOURCES
      ${_SOURCE_DIR}/${_SOURCE})
  endforeach()

  #----------------------------------------------------------------------------#
  # Add top-level source directory.
  #----------------------------------------------------------------------------#

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${directory}/..)

  #----------------------------------------------------------------------------#
  # Add subdirectories
  #
  # This uses a glob, i.e., all sub-directories will be added at this level.
  # This is not true for levels below this one.  This allows some flexibility
  # while keeping the generic case as simple as possible.
  #----------------------------------------------------------------------------#

  make_subdirlist(_SUBDIRECTORIES ${_SOURCE_DIR} False)

  #----------------------------------------------------------------------------#
  # Add subdirectory files
  #----------------------------------------------------------------------------#
  
  # This loop adds header and source files for each listed sub-directory
  # to the main header and source file lists.  Additionally, it adds the
  # listed sub-directories to the include search path.  Lastly, it creates
  # a catalog for each sub-directory with information on the source and
  # headers files from the directory using the 'info.cmake' script that is
  # located in the top-level 'cmake' directory.
  
  foreach(_SUBDIR ${_SUBDIRECTORIES})
 
    if(NOT EXISTS ${_SOURCE_DIR}/${_SUBDIR}/CMakeLists.txt)
     continue()
    endif()
  
    message(STATUS
      "Adding source subdirectory '${_SUBDIR}' to ${target}")
  
    unset(${_SUBDIR}_HEADERS)
    unset(${_SUBDIR}_SOURCES)
  
    add_subdirectory(${directory}/${_SUBDIR})
  
    foreach(_HEADER ${${_SUBDIR}_HEADERS})
      if(NOT EXISTS ${_SOURCE_DIR}/${_SUBDIR}/${_HEADER})
        message(FATAL_ERROR "Header '${_HEADER}' from ${_SUBDIR}_HEADERS does not exist.")
      endif()
      list(APPEND HEADERS
        ${_SUBDIR}/${_HEADER})
      list(APPEND GLOBAL_HEADERS ${_SOURCE_DIR}/${_SUBDIR}/${_HEADER})
    endforeach()
  
    foreach(_SOURCE ${${_SUBDIR}_SOURCES})
      list(APPEND SOURCES
        ${_SOURCE_DIR}/${_SUBDIR}/${_SOURCE})
    endforeach()
  
  endforeach(_SUBDIR)
 
  # if there are source files, build a library
  if(SOURCES)
   add_library(${target} ${SOURCES})
   # if an export target has been specified
   if(lib_EXPORT_TARGET)
    install(TARGETS ${target} EXPORT ${lib_EXPORT_TARGET}
      DESTINATION ${LIBDIR})
   else()
    install(TARGETS ${target} DESTINATION ${LIBDIR})
   endif()
  # else this is a header only interface
  else()
   add_library(${target} INTERFACE)
  endif()

  foreach(file ${HEADERS})
    get_filename_component(DIR ${file} DIRECTORY)
    install(FILES ${directory}/${file}
      DESTINATION include/${directory}/${DIR})
  endforeach()

  foreach(file ${${target}_PUBLIC_HEADERS})
    install(FILES ${directory}/${file} DESTINATION include)
  endforeach()
endfunction()

function(add_target_link_libraries target)

  if (ARGN)
    get_target_property(_type ${target} TYPE)
    if ( ${_type} STREQUAL "INTERFACE_LIBRARY")
      target_link_libraries( ${target} INTERFACE ${ARGN} )
    else()
	    target_link_libraries( ${target} INTERFACE ${ARGN} )
    endif()
  endif()

endfunction()
