#------------------------------------------------------------------------------#
#   @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
#  /@@/////  /@@          @@////@@ @@////// /@@
#  /@@       /@@  @@@@@  @@    // /@@       /@@
#  /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
#  /@@////   /@@/@@@@@@@/@@       ////////@@/@@
#  /@@       /@@/@@//// //@@    @@       /@@/@@
#  /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
#  //       ///  //////   //////  ////////  //
#
#  Copyright (c) 2016, Los Alamos National Security, LLC
#  All rights reserved.
#------------------------------------------------------------------------------#

include(CMakeDependentOption)

cmake_dependent_option(ENABLE_UNIT_TESTS "Enalle unit testing" ON
  "ENABLE_FLOG" OFF)
cmake_dependent_option(ENABLE_EXPENSIVE_TESTS
  "Enalle unit tests labeled 'expensive'" OFF "ENABLE_FLOG" OFF)

mark_as_advanced(ENABLE_EXPENSIVE_TESTS)

if(ENABLE_UNIT_TESTS)
  enable_testing()

  if(FLECSI_RUNTIME_MODEL STREQUAL "charm")
    # Ensure that decl and def headers are generated before tests are compiled
    add_library(unit-main OBJECT
      ${CMAKE_SOURCE_DIR}/flecsi/util/unit/main.cc ${all-ci-outputs})
  else()
    add_library(unit-main OBJECT ${CMAKE_SOURCE_DIR}/flecsi/util/unit/main.cc)
  endif()
endif()

function(add_unit name)

  #----------------------------------------------------------------------------#
  # Enable new behavior for in-list if statements.
  #----------------------------------------------------------------------------#

  cmake_policy(SET CMP0057 NEW)

  if(NOT ENABLE_UNIT_TESTS)
    return()
  endif()

  #----------------------------------------------------------------------------#
  # Setup argument options.
  #----------------------------------------------------------------------------#

  set(options NOCI NOOPENMPI)
  set(one_value_args POLICY)
  set(multi_value_args
    SOURCES INPUTS THREADS LIBRARIES DEFINES DRIVER ARGUMENTS TESTLABELS
  )
  cmake_parse_arguments(unit "${options}" "${one_value_args}"
    "${multi_value_args}" ${ARGN})

  #----------------------------------------------------------------------------#
  # Is this an expensive test? If so, and if this build does not enable
  # expensive tests, then skip it
  #----------------------------------------------------------------------------#

  if("expensive" IN_LIST unit_TESTLABELS)
    if(NOT "${ENABLE_EXPENSIVE_TESTS}")
      return()
    endif()
  endif()

  #----------------------------------------------------------------------------#
  # Set output directory
  #----------------------------------------------------------------------------#

  get_filename_component(_SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    set(_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test/${_SOURCE_DIR_NAME}")
  else()
    set(_OUTPUT_DIR
      "${CMAKE_BINARY_DIR}/test/${PROJECT_NAME}/${_SOURCE_DIR_NAME}")
  endif()

  #----------------------------------------------------------------------------#
  # Make sure that MPI_LANGUAGE is set.
  # This is not a standard variable set by FindMPI, but cinch might set it.
  #
  # Right now, the MPI policy only works with C/C++.
  #----------------------------------------------------------------------------#

  if(NOT MPI_LANGUAGE)
    set(MPI_LANGUAGE C)
  endif()

  #----------------------------------------------------------------------------#
  # Set output directory information.
  #----------------------------------------------------------------------------#

  if("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
      set(_TEST_PREFIX)
  else()
      set(_TEST_PREFIX "${PROJECT_NAME}:")
  endif()

  #----------------------------------------------------------------------------#
  # Check to see if the user has specified a runtime and process it.
  #----------------------------------------------------------------------------#
  if(FLECSI_RUNTIME_MODEL STREQUAL "mpi"
    AND MPI_${MPI_LANGUAGE}_FOUND)

    set(unit_policy_flags ${MPI_${MPI_LANGUAGE}_COMPILE_FLAGS})
    set(unit_policy_includes ${MPI_${MPI_LANGUAGE}_INCLUDE_PATH})
    set(unit_policy_libraries ${MPI_${MPI_LANGUAGE}_LIBRARIES})
    set(unit_policy_exec ${MPIEXEC})
    set(unit_policy_exec_threads ${MPIEXEC_NUMPROC_FLAG})
    set(unit_policy_exec_preflags ${MPIEXEC_PREFLAGS})
    set(unit_policy_exec_postflags ${MPIEXEC_POSTFLAGS})

  elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion"
    AND MPI_${MPI_LANGUAGE}_FOUND
    AND Legion_FOUND)

    set(unit_policy_flags ${Legion_CXX_FLAGS}
      ${MPI_${MPI_LANGUAGE}_COMPILE_FLAGS})
    set(unit_policy_includes ${Legion_INCLUDE_DIRS}
      ${MPI_${MPI_LANGUAGE}_INCLUDE_PATH})
    set(unit_policy_libraries ${Legion_LIBRARIES} ${Legion_LIB_FLAGS}
      ${MPI_${MPI_LANGUAGE}_LIBRARIES})
    set(unit_policy_exec ${MPIEXEC})
    set(unit_policy_exec_threads ${MPIEXEC_NUMPROC_FLAG})
    set(unit_policy_exec_preflags ${MPIEXEC_PREFLAGS})
    set(unit_policy_exec_postflags ${MPIEXEC_POSTFLAGS})

  elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm"
    AND MPI_${MPI_LANGUAGE}_FOUND)

    set(unit_policy_flags ${MPI_${MPI_LANGUAGE}_COMPILE_FLAGS})
    set(unit_policy_includes ${MPI_${MPI_LANGUAGE}_INCLUDE_PATH})
    set(unit_policy_libraries ${MPI_${MPI_LANGUAGE}_LIBRARIES})
    set(unit_policy_exec ${MPIEXEC})
    set(unit_policy_exec_threads ${MPIEXEC_NUMPROC_FLAG})
    set(unit_policy_exec_preflags ${MPIEXEC_PREFLAGS})
    set(unit_policy_exec_postflags ${MPIEXEC_POSTFLAGS})

  else()

    message(WARNING "invalid runtime")
    return()

  endif()

  #----------------------------------------------------------------------------#
  # Make sure that the user specified sources.
  #----------------------------------------------------------------------------#

  if(NOT unit_SOURCES)
    message(FATAL_ERROR
      "You must specify unit test source files using SOURCES")
  endif()

  #----------------------------------------------------------------------------#
  # Set file properties for FleCSI language files.
  #----------------------------------------------------------------------------#

  foreach(source ${unit_SOURCES})
    # Identify FleCSI language source files and add the appropriate
    # language and compiler flags to properties.

    get_filename_component(_EXT ${source} EXT)

    if("${_EXT}" STREQUAL ".fcc")
      set_source_files_properties(${source} PROPERTIES LANGUAGE CXX
    )
    endif()
  endforeach()

  add_executable(${name}
    ${unit_SOURCES}
    $<TARGET_OBJECTS:unit-main>
  )
  
  set_target_properties(${name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${_OUTPUT_DIR})

  if(unit_policy_flags)
    set( unit_policy_list ${unit_policy_flags} )
    separate_arguments(unit_policy_list)

    target_compile_options(${name} PRIVATE ${unit_policy_list})
  endif()

  #----------------------------------------------------------------------------#
  # Set the folder property for VS and XCode
  #----------------------------------------------------------------------------#

  get_filename_component(_leafdir ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  string(SUBSTRING ${_leafdir} 0 1 _first)
  string(TOUPPER ${_first} _first)
  string(REGEX REPLACE "^.(.*)" "${_first}\\1" _leafdir "${_leafdir}")
  string(CONCAT _folder "Tests/" ${_leafdir})
  set_target_properties(${name} PROPERTIES FOLDER "${_folder}")

  #----------------------------------------------------------------------------#
  # Check for defines.
  #----------------------------------------------------------------------------#

  if(unit_policy_defines)
    target_compile_definitions(${name} PRIVATE ${unit_policy_defines})
  endif()

  if(unit_DEFINES)
    target_compile_definitions(${name} PRIVATE ${unit_DEFINES})
  endif()

  #----------------------------------------------------------------------------#
  # Check for input files.
  #----------------------------------------------------------------------------#

  if(unit_INPUTS)
    set(_OUTPUT_FILES)
    foreach(input ${unit_INPUTS})
      get_filename_component(_OUTPUT_NAME ${input} NAME)
      get_filename_component(_PATH ${input} ABSOLUTE)
      configure_file(${_PATH} ${_OUTPUT_DIR}/${_OUTPUT_NAME} COPYONLY)
      list(APPEND _OUTPUT_FILES ${_OUTPUT_DIR}/${_OUTPUT_NAME})
    endforeach()
    add_custom_target(${name}_inputs
      DEPENDS ${_OUTPUT_FILES})
    set_target_properties(${name}_inputs
      PROPERTIES FOLDER "${_folder}/Inputs")
    add_dependencies(${name} ${name}_inputs)
  endif()

  #----------------------------------------------------------------------------#
  # Check for library dependencies.
  #----------------------------------------------------------------------------#

  if(unit_LIBRARIES)
    target_link_libraries(${name} ${unit_LIBRARIES})
  endif()

  target_link_libraries(${name} FleCSI)
  target_link_libraries(${name} ${FLECSI_LIBRARY_DEPENDENCIES}
    ${CMAKE_THREAD_LIBS_INIT})

  if(unit_policy_libraries)
    target_link_libraries(${name} ${unit_policy_libraries})
  endif()

  if(unit_policy_includes)
      target_include_directories(${name}
          PRIVATE ${unit_policy_includes})
  endif()

  #----------------------------------------------------------------------------#
  # Check for threads.
  #
  # If found, replace the semi-colons with pipes to avoid list
  # interpretation.
  #----------------------------------------------------------------------------#

  if(NOT unit_THREADS)
    set(unit_THREADS 1)
  endif(NOT unit_THREADS)

  #----------------------------------------------------------------------------#
  # Add the test target to CTest
  #----------------------------------------------------------------------------#

  if(unit_NOOPENMPI AND ( "$ENV{OPENMPI}" STREQUAL "true" )
    AND ( NOT "$ENV{IGNORE_NOOPENMPI}" STREQUAL "true" ))
    message(STATUS "Skipping test ${_TEST_PREFIX}${name} "
      " due to OPENMPI enabled")
    return()
  endif()

  if(unit_NOCI AND ( "$ENV{CI}" STREQUAL "true" )
    AND ( NOT "$ENV{IGNORE_NOCI}" STREQUAL "true" ))
    message(STATUS "Skipping test ${_TEST_PREFIX}${name} due to CI enabled")
    return()
  endif()

  list(LENGTH unit_THREADS thread_instances)

  if(${thread_instances} GREATER 1)
    foreach(instance ${unit_THREADS})
      add_test(
        NAME
          "${_TEST_PREFIX}${name}_${instance}"
        COMMAND
          ${unit_policy_exec}
          ${unit_policy_exec_preflags}
          ${unit_policy_exec_threads} ${instance}
          $<TARGET_FILE:${name}>
          ${unit_ARGUMENTS}
          ${unit_policy_exec_postflags}
          ${UNIT_FLAGS}
        WORKING_DIRECTORY ${_OUTPUT_DIR}
      )

      set_tests_properties("${_TEST_PREFIX}${name}_${instance}"
        PROPERTIES LABELS "${unit_TESTLABELS}"
      )
    endforeach()
  else()
    if(unit_policy_exec)
      add_test(
        NAME
          "${_TEST_PREFIX}${name}"
        COMMAND
          ${unit_policy_exec}
          ${unit_policy_exec_threads}
          ${unit_THREADS}
          ${unit_policy_exec_preflags}
          $<TARGET_FILE:${name}>
          ${unit_ARGUMENTS}
          ${unit_policy_exec_postflags}
          ${UNIT_FLAGS}
        WORKING_DIRECTORY ${_OUTPUT_DIR}
      )
  else()
      add_test(
        NAME
          "${_TEST_PREFIX}${name}"
        COMMAND
          $<TARGET_FILE:${name}>
          ${UNIT_FLAGS}
        WORKING_DIRECTORY ${_OUTPUT_DIR}
      )
    endif()

    set_tests_properties("${_TEST_PREFIX}${name}" PROPERTIES
      LABELS "${unit_TESTLABELS}")

  endif()

endfunction()
