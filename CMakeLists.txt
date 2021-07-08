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
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.12)

#------------------------------------------------------------------------------#
# Add local module path.
#------------------------------------------------------------------------------#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------#
# Project.
#------------------------------------------------------------------------------#

project(FleCSI LANGUAGES CXX C)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------#
# Compiler version requirements.
#------------------------------------------------------------------------------#

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    message(FATAL_ERROR "Version 9.0 of gnu compilers required!")
  endif()
endif()

#------------------------------------------------------------------------------#
# Version.
#------------------------------------------------------------------------------#

include(version)

set(FleCSI_VERSION ${${PROJECT_NAME}_VERSION})

#------------------------------------------------------------------------------#
# Add summary macros.
#------------------------------------------------------------------------------#

include(summary)

#------------------------------------------------------------------------------#
# Add format target.
#------------------------------------------------------------------------------#

include(format)

if(NOT FORMAT_ONLY)

  #----------------------------------------------------------------------------#
  # Add spotless target
  #----------------------------------------------------------------------------#

  add_custom_target(spotless rm -rf ${CMAKE_BINARY_DIR}/*)

  #----------------------------------------------------------------------------#
  # Add option for setting entity id type 
  #----------------------------------------------------------------------------#

  set(FLECSI_ID_TYPE "std::uint32_t" CACHE STRING
    "Select the type that will be used for entity ids by topologies")
  mark_as_advanced(FLECSI_ID_TYPE)

  #----------------------------------------------------------------------------#
  # Add option for counter size
  #----------------------------------------------------------------------------#

  set(FLECSI_COUNTER_TYPE "int32_t" CACHE STRING
    "Select the type that will be used for loop and iterator values")
  mark_as_advanced(FLECSI_COUNTER_TYPE)

  #----------------------------------------------------------------------------#
  # These variables are used to collect library and include dependencies
  # for the FleCSIConfig file below.
  #----------------------------------------------------------------------------#

  set(FLECSI_INCLUDE_DEPENDENCIES)
  set(FLECSI_LIBRARY_DEPENDENCIES)

  #----------------------------------------------------------------------------#
  # Coverage.
  #----------------------------------------------------------------------------#

  include(coverage)

  #----------------------------------------------------------------------------#
  # Add options for runtime selection
  #----------------------------------------------------------------------------#
  set(FLECSI_RUNTIME_MODELS legion mpi hpx charm)

  if(NOT FLECSI_RUNTIME_MODEL)
    list(GET FLECSI_RUNTIME_MODELS 0 FLECSI_RUNTIME_MODEL)
  endif()

  set(FLECSI_RUNTIME_MODEL "${FLECSI_RUNTIME_MODEL}" CACHE STRING
    "Select the runtime model")
  set_property(CACHE FLECSI_RUNTIME_MODEL
    PROPERTY STRINGS ${FLECSI_RUNTIME_MODELS})

  #----------------------------------------------------------------------------#
  # Runtime defaults.
  #----------------------------------------------------------------------------#

  if(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
    set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
    set(ENABLE_LEGION OFF CACHE BOOL "Enable Legion" FORCE)
  elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")
    set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
    set(ENABLE_LEGION ON CACHE BOOL "Enable Legion" FORCE)
  elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
    set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
    set(ENABLE_HPX ON CACHE BOOL "Enable HPX" FORCE)
  elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")
    set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
    set(ENABLE_CHARM ON CACHE BOOL "Enable Charm" FORCE)
    set(ENABLE_LEGION OFF CACHE BOOL "Enable Legion" FORCE)
  endif()

  mark_as_advanced(ENABLE_MPI ENABLE_LEGION ENABLE_HPX ENABLE_CHARM)

  #----------------------------------------------------------------------------#
  # Legion
  #----------------------------------------------------------------------------#

  if(ENABLE_LEGION)
    include(legion)
  endif()

  #----------------------------------------------------------------------------#
  # MPI
  #----------------------------------------------------------------------------#

  if(ENABLE_MPI)
    include(mpi)
  endif()

  #----------------------------------------------------------------------------#
  # HPX
  #----------------------------------------------------------------------------#

  if(ENABLE_HPX)
    include(hpx)
  endif()

  #----------------------------------------------------------------------------#
  # Charm
  #----------------------------------------------------------------------------#

  if(ENABLE_CHARM)
    include(charm)
  endif()

  #----------------------------------------------------------------------------#
  # OpenMP.
  #----------------------------------------------------------------------------#

  include(openmp)

  #----------------------------------------------------------------------------#
  # HDF5.
  #----------------------------------------------------------------------------#

  include(hdf5)

  #----------------------------------------------------------------------------#
  # Documentation.
  #----------------------------------------------------------------------------#

  include(documentation)

  #----------------------------------------------------------------------------#
  # Graphviz
  #----------------------------------------------------------------------------#

  include(graphviz)

  #----------------------------------------------------------------------------#
  # Boost.
  #----------------------------------------------------------------------------#

  set(ENABLE_BOOST ON CACHE BOOL "Enable Boost (required by FleCSI)" FORCE)
  mark_as_advanced(ENABLE_BOOST)

  include(boost)

  #----------------------------------------------------------------------------#
  # Kokkos
  #----------------------------------------------------------------------------#

  include(kokkos)

  #----------------------------------------------------------------------------#
  # Caliper Annotation options
  #----------------------------------------------------------------------------#

  include(annotation)

  #----------------------------------------------------------------------------#
  # Runtime models
  #----------------------------------------------------------------------------#

  set(FLECSI_RUNTIME_LIBRARIES)

  set(DL_LIBS)
  foreach(dl_lib ${CMAKE_DL_LIBS})
    find_library(DL_LIB ${dl_lib})

    if(DL_LIB STREQUAL "NOTFOUND")
      message(FATAL_ERROR "Dynamic library not found")
    endif()

    list(APPEND DL_LIBS ${DL_LIB})
  endforeach()

  mark_as_advanced(DL_LIB)

  #----------------------------------------------------------------------------#
  # Legion interface
  #----------------------------------------------------------------------------#

  if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

    if(NOT MPI_${MPI_LANGUAGE}_FOUND)
      message (FATAL_ERROR "MPI is required for the legion runtime model")
    endif()

    if(NOT Legion_FOUND)
      message (FATAL_ERROR "Legion is required for the legion runtime model")
    endif()

    set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/legion)

    set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${Legion_LIBRARIES}
      ${MPI_LIBRARIES})

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${Legion_INCLUDE_DIRS})

    option(ENABLE_MAPPER_COMPACTION "Enable Legion Mapper compaction" ON)
    mark_as_advanced(ENABLE_MAPPER_COMPACTION)

  #----------------------------------------------------------------------------#
  # MPI interface
  #----------------------------------------------------------------------------#

  elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

    if(NOT MPI_${MPI_LANGUAGE}_FOUND)
      message (FATAL_ERROR "MPI is required for the mpi runtime model")
    endif()

    set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/mpi)

    set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${MPI_LIBRARIES})

  #----------------------------------------------------------------------------#
  # HPX interface
  #----------------------------------------------------------------------------#

  elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

    if(NOT HPX_FOUND)
      message (FATAL_ERROR "HPX is required for the HPX runtime model")
    endif()

    if(NOT MPI_${MPI_LANGUAGE}_FOUND)
      message (FATAL_ERROR "MPI is required for the hpx runtime model")
    endif()

    set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${MPI_LIBRARIES})

    set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/hpx)

  #----------------------------------------------------------------------------#
  # Charm interface
  #----------------------------------------------------------------------------#
  elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")

    if(NOT MPI_${MPI_LANGUAGE}_FOUND)
      message (FATAL_ERROR "MPI is required for the charm runtime model")
    endif()

    set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/charm)

    set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${MPI_LIBRARIES})

  #----------------------------------------------------------------------------#
  # Default
  #----------------------------------------------------------------------------#

  else()

    message(FATAL_ERROR "Unrecognized runtime selection")

  endif()

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${FLECSI_RUNTIME_LIBRARIES})

  #----------------------------------------------------------------------------#
  # Enable partitioning with METIS
  #----------------------------------------------------------------------------#

  find_package(METIS 5.1)

  if(ENABLE_MPI)
    # Counter-intuitive variable: set to TRUE to disable test
    set(PARMETIS_TEST_RUNS TRUE)
    find_package(ParMETIS 4.0)
  endif()

  set(COLORING_LIBRARIES)

  if(METIS_FOUND)
    list(APPEND COLORING_LIBRARIES ${METIS_LIBRARIES})
    include_directories(SYSTEM ${METIS_INCLUDE_DIRS})
    set(ENABLE_METIS TRUE)

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${METIS_INCLUDE_DIRS})
  endif()

  if(PARMETIS_FOUND)
    list(APPEND COLORING_LIBRARIES ${PARMETIS_LIBRARIES})
    include_directories(SYSTEM ${PARMETIS_INCLUDE_DIRS})
    set(ENABLE_PARMETIS TRUE)

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${PARMETIS_INCLUDE_DIRS})
  endif()

  if(NOT COLORING_LIBRARIES)
    MESSAGE(FATAL_ERROR
      "You need parmetis to enable partitioning" )
  endif()

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${COLORING_LIBRARIES})

  #----------------------------------------------------------------------------#
  # FLOG and FTEST
  #----------------------------------------------------------------------------#

  include(flog)
  include(unit)

  #----------------------------------------------------------------------------#
  # Capture enable settings.
  #----------------------------------------------------------------------------#

  get_cmake_property(_variableNames VARIABLES)
  string (REGEX MATCHALL "(^|;)ENABLE_[A-Za-z0-9_]*"
    _matchedVars "${_variableNames}")

  foreach(_variableName ${_matchedVars})
    set(FLECSI_${_variableName} ${${_variableName}})
  endforeach()

  #----------------------------------------------------------------------------#
  # Configure header.
  #----------------------------------------------------------------------------#

  configure_file(${PROJECT_SOURCE_DIR}/config/flecsi-config.h.in
    ${CMAKE_BINARY_DIR}/flecsi-config.h @ONLY)

  include_directories(${CMAKE_BINARY_DIR})

  install(
    FILES ${CMAKE_BINARY_DIR}/flecsi-config.h
    DESTINATION include
  )


  #----------------------------------------------------------------------------#
  # mesh-gen target
  #----------------------------------------------------------------------------#

  add_executable(mesh-gen tools/mesh-gen.cc)

  #----------------------------------------------------------------------------#
  # Configure environment module.
  #----------------------------------------------------------------------------#

  configure_file(${PROJECT_SOURCE_DIR}/config/flecsi-module.in
    ${CMAKE_BINARY_DIR}/flecsi-module @ONLY)

  #----------------------------------------------------------------------------#
  # FleCSI library.
  #----------------------------------------------------------------------------#

  include(library)

  add_library_target(FleCSI flecsi EXPORT_TARGET FleCSITargets)

  #----------------------------------------------------------------------------#
  # Prepare variables for FleCSIConfig file.
  #----------------------------------------------------------------------------#

  set(FLECSI_EXTERNAL_INCLUDE_DIRS)

  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY INCLUDE_DIRECTORIES)

  foreach(dir ${dirs})
    if(NOT ${dir} MATCHES ${CMAKE_CURRENT_SOURCE_DIR})
      list(APPEND FLECSI_EXTERNAL_INCLUDE_DIRS ${dir})
    endif()
  endforeach()

  set(FLECSI_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${LIBDIR})
  set(FLECSI_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include
    ${FLECSI_EXTERNAL_INCLUDE_DIRS})

  set(FLECSI_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/cmake/FleCSI)

  #----------------------------------------------------------------------------#
  # Extract all project options so they can be exported to the
  # ProjectConfig.cmake file.
  #----------------------------------------------------------------------------#

  get_cmake_property(_variableNames VARIABLES)
  string (REGEX MATCHALL "(^|;)FLECSI_[A-Za-z0-9_]*"
    _matchedVars "${_variableNames}")

  foreach(_variableName ${_matchedVars})
    string(REGEX REPLACE "FLECSI" "FleCSI" _tmpvariableName ${_variableName})
    set(FLECSI_CONFIG_CODE
      "${FLECSI_CONFIG_CODE}\nset(${_tmpvariableName} \"${${_variableName}}\")")
  endforeach()

  string(REGEX MATCHALL "(^|;)FLOG_[A-Za-z0-9_]*"
    _matchedVars "${_variableNames}")

  foreach(_variableName ${_matchedVars})
    set(FLOG_CONFIG_CODE
      "${FLOG_CONFIG_CODE}\nset(${_variableName} \"${${_variableName}}\")")
  endforeach()

  #----------------------------------------------------------------------------#
  # Export targets and package.
  #----------------------------------------------------------------------------#

  export(
    TARGETS FleCSI
    FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSITargets.cmake
  )

  export(PACKAGE FleCSI)

  #----------------------------------------------------------------------------#
  # Link necessary libraries.
  #----------------------------------------------------------------------------#

  if(FLECSI_RUNTIME_LIBRARIES OR COLORING_LIBRARIES)
    add_target_link_libraries(
      FleCSI ${FLECSI_RUNTIME_LIBRARIES} ${COLORING_LIBRARIES}
    )
  endif()

  if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

    hpx_setup_target(FleCSI NONAMEPREFIX)

  endif()

  #----------------------------------------------------------------------------#
  # CMake config file: This should be the last thing to happen.
  #----------------------------------------------------------------------------#

  configure_file(${PROJECT_SOURCE_DIR}/config/FleCSIConfig.cmake.in
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSIConfig.cmake @ONLY)

  install(
    FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSIConfig.cmake
    DESTINATION ${FLECSI_CMAKE_DIR}
  )

  install(
    EXPORT FleCSITargets
    DESTINATION ${FLECSI_CMAKE_DIR}
    COMPONENT dev
  )

  #----------------------------------------------------------------------------#
  # Output configuration summary.
  #----------------------------------------------------------------------------#

  summary_header()
  summary_info("CMAKE_BUILD_TYPE" "${CMAKE_BUILD_TYPE}" TRUE)
  summary_info("CMAKE_INSTALL_PREFIX" "${CMAKE_INSTALL_PREFIX}" TRUE)
  string(APPEND _summary "\n")
  summary_info("CMAKE_CXX_COMPILER" "${CMAKE_CXX_COMPILER}" TRUE)
  summary_info("CMAKE_CXX_FLAGS" "${CMAKE_CXX_FLAGS}" TRUE)
  summary_info("CMAKE_C_COMPILER" "${CMAKE_C_COMPILER}" TRUE)
  summary_info("CMAKE_C_FLAGS" "${CMAKE_C_FLAGS}" TRUE)
  if(ClangFormat_FOUND)
    summary_info("ClangFormat_EXECUTABLE" "${ClangFormat_EXECUTABLE}" TRUE)
  endif()
  string(APPEND _summary "\n")
  summary_info("FleCSI_VERSION" "${FleCSI_VERSION}" TRUE)
  summary_info("FleCSI_DOCUMENTATION_VERSION"
    "${FleCSI_DOCUMENTATION_VERSION}" FALSE)
  if(${PROJECT_NAME}_COMMITS)
    summary_info("Repository state" "${${PROJECT_NAME}_COMMITS}" FALSE)
  endif()
  summary_info("FLECSI_RUNTIME_MODEL" "${FLECSI_RUNTIME_MODEL}" TRUE)
  summary_option("ENABLE_FLOG" ${ENABLE_FLOG}
    " (FLOG_STRIP_LEVEL ${FLOG_STRIP_LEVEL})")
  summary_option("ENABLE_UNIT_TESTS" ${ENABLE_UNIT_TESTS} "")
  string(APPEND _summary "\n")
  summary_option("ENABLE_KOKKOS" ${ENABLE_KOKKOS}
    " (${FLECSI_NODE_PLATFORM} platform)")
  if(ENABLE_DOCUMENTATION)
    summary_option("ENABLE_SPHINX" ${ENABLE_SPHINX} " (ENABLE_DOCUMENTATION)")
    summary_option("ENABLE_DOXYGEN" ${ENABLE_DOXYGEN} " (ENABLE_DOCUMENTATION)")
  endif()
  summary_option("ENABLE_GRAPHVIZ" ${ENABLE_GRAPHVIZ} "")
  summary_option("ENABLE_OPENMP" ${ENABLE_OPENMP} "")
  summary_option("ENABLE_HDF5" ${ENABLE_HDF5} "")
  summary_info("FLECSI_CALIPER_DETAIL" "${FLECSI_CALIPER_DETAIL}" TRUE)

  message(STATUS ${_summary})

else()

  summary_header()
  summary_option("FORMAT_ONLY" ${FORMAT_ONLY} "")

  message(STATUS ${_summary})

endif()
