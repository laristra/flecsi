#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# If a C++14 compiler is available, then set the appropriate flags
#------------------------------------------------------------------------------#

include(cxx14)

check_for_cxx14_compiler(CXX14_COMPILER)

if(CXX14_COMPILER)
    enable_cxx14()
else()
    message(FATAL_ERROR "C++14 compatible compiler not found")
endif()

set(FLECSI_RUNTIME_LIBRARIES)

#------------------------------------------------------------------------------#
# Find Legion
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion" OR
  FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  find_package(Legion REQUIRED)

  message(STATUS "Legion found: ${Legion_FOUND}")

endif()

#------------------------------------------------------------------------------#
# Runtime models
#------------------------------------------------------------------------------#

#
# Serial interface
#
if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  add_definitions(-DFLECSI_RUNTIME_MODEL_serial)
  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/serial)

#
# Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  add_definitions(-DFLECSI_RUNTIME_MODEL_legion)
  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/legion)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES  ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  endif()

  include_directories(${Legion_INCLUDE_DIRS})

#
# MPI interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  add_definitions(-DFLECSI_RUNTIME_MODEL_mpi)
  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/mpi)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES ${MPI_LIBRARIES})
  endif()

#
# MPI+Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  if(NOT ENABLE_MPI)
    message (FATAL_ERROR "MPI is required for the mpilegion runtime model")
  endif()
 
  add_definitions(-DFLECSI_RUNTIME_MODEL_mpilegion)
  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/mpilegion)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES  ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  endif()

  include_directories(${Legion_INCLUDE_DIRS})

#
# Default
#
else(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif(FLECSI_RUNTIME_MODEL STREQUAL "serial")

#------------------------------------------------------------------------------#
# Hypre
#------------------------------------------------------------------------------#

set(ENABLE_HYPRE OFF CACHE BOOL " do you want to enable HYPRE?")

if(ENABLE_HYPRE)
  find_package (HYPRE)

 if(HYPRE_FOUND)
   include_directories(${HYPRE_INCLUDE_DIRS})
   set(HYPRE_LIBRARY ${HYPRE_LIBRARIES})
 else()
   message (ERROR "HYPRE required for this build is not found")
  endif()

endif(ENABLE_HYPRE)

#------------------------------------------------------------------------------#
# Cereal
#------------------------------------------------------------------------------#

  find_package (Cereal REQUIRED)
  include_directories(${Cereal_INCLUDE_DIRS})

#------------------------------------------------------------------------------#
# Process id bits
#------------------------------------------------------------------------------#

# PBITS: possible number of distributed-memory partitions
# EBITS: possible number of entities per partition
# GBITS: possible number of global ids
# FBITS: flag bits
# dimension: dimension 2 bits
# domain: dimension 2 bits

# Make sure that the user set FBITS to an even number
math(EXPR FLECSI_EVEN_FBITS "${FLECSI_ID_FBITS} % 2")
if(NOT ${FLECSI_EVEN_FBITS} EQUAL 0)
  message(FATAL_ERROR "FLECSI_ID_FBITS must be an even number")
endif()

# Get the total number of bits left for ids
math(EXPR FLECSI_ID_BITS "124 - ${FLECSI_ID_FBITS}")

# Global ids use half of the remaining bits
math(EXPR FLECSI_ID_GBITS "${FLECSI_ID_BITS}/2")

# EBITS and PBITS must add up to GBITS
math(EXPR FLECSI_ID_EBITS "${FLECSI_ID_GBITS} - ${FLECSI_ID_PBITS}")

add_definitions(-DFLECSI_ID_PBITS=${FLECSI_ID_PBITS})
add_definitions(-DFLECSI_ID_EBITS=${FLECSI_ID_EBITS})
add_definitions(-DFLECSI_ID_FBITS=${FLECSI_ID_FBITS})
add_definitions(-DFLECSI_ID_GBITS=${FLECSI_ID_GBITS})

math(EXPR flecsi_partitions "1 << ${FLECSI_ID_PBITS}")
math(EXPR flecsi_entities "1 << ${FLECSI_ID_EBITS}")

message(STATUS "${CINCH_Yellow}Set id_t bits to allow:\n"
  "   ${flecsi_partitions} partitions with 2^${FLECSI_ID_EBITS} entities each\n"
  "   ${FLECSI_ID_FBITS} flag bits\n"
  "   ${FLECSI_ID_GBITS} global bits (PBITS*EBITS)${CINCH_ColorReset}")

#------------------------------------------------------------------------------#
# Counter type
#------------------------------------------------------------------------------#

add_definitions(-DFLECSI_COUNTER_TYPE=${FLECSI_COUNTER_TYPE})

#------------------------------------------------------------------------------#
# Enable IO with exodus
#------------------------------------------------------------------------------#

find_package(EXODUSII)
option(ENABLE_IO "Enable I/O (uses libexodus)" ${EXODUSII_FOUND})

if(ENABLE_IO)

  if(EXODUSII_FOUND)
    set(IO_LIBRARIES ${EXODUSII_LIBRARIES})
    include_directories(${EXODUSII_INCLUDE_DIRS})
  else()
    MESSAGE(FATAL_ERROR "You need libexodus either from TPL "
      "or system to enable I/O")
  endif()
  add_definitions( -DHAVE_EXODUS )

endif(ENABLE_IO)

#------------------------------------------------------------------------------#
# Enable partitioning with METIS or SCOTCH
#------------------------------------------------------------------------------#

find_package(METIS 5.1)
find_package(SCOTCH)

if(ENABLE_MPI)
  # Counter-intuitive variable: set to TRUE to disable test
  set(PARMETIS_TEST_RUNS TRUE)
  find_package(ParMETIS 4.0)
endif()

if(PARMETIS_FOUND OR SCOTCH_FOUND)
  option(ENABLE_PARTITION
    "Enable partitioning (uses metis/parmetis or scotch)." ON)
else()
  option(ENABLE_PARTITION
    "Enable partitioning (uses metis/parmetis or scotch)." OFF)
endif()

if(ENABLE_PARTITION)

  set( PARTITION_LIBRARIES )

  if(METIS_FOUND)
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARIES} )
     include_directories( ${METIS_INCLUDE_DIRS} )
     add_definitions( -DHAVE_METIS )
  endif()

  if(PARMETIS_FOUND)
    message(STATUS "Adding parmetis ${PARMETIS_INCLUDE_DIRS}")
    list( APPEND PARTITION_LIBRARIES ${PARMETIS_LIBRARIES} )
    include_directories( ${PARMETIS_INCLUDE_DIRS} )
    add_definitions( -DHAVE_PARMETIS )
  endif()

  if(SCOTCH_FOUND)
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARIES} )
     include_directories( ${SCOTCH_INCLUDE_DIRS} )
     add_definitions( -DHAVE_SCOTCH )
     if(SCOTCH_VERSION MATCHES ^5)
       #SCOTCH_VERSION from scotch.h is broken in scotch-5
       add_definitions( -DHAVE_SCOTCH_V5 )
     endif(SCOTCH_VERSION MATCHES ^5)
  endif()

  if(NOT PARTITION_LIBRARIES)
    MESSAGE(FATAL_ERROR
      "You need scotch, metis or parmetis to enable partitioning" )
  endif()

endif()

#------------------------------------------------------------------------------#
# LAPACK
#------------------------------------------------------------------------------#

# NB: The code that uses lapack actually requires lapacke:
# http://www.netlib.org/lapack/lapacke.html
# If the installation of lapack that this finds does not contain lapacke then
# the build will fail.
if(NOT APPLE)
  find_package(LAPACKE)

  if(LAPACKE_FOUND)
      include_directories( ${LAPACKE_INCLUDE_DIRS} )
  endif(LAPACKE_FOUND)
endif(NOT APPLE)

#------------------------------------------------------------------------------#
# Collect information for FleCSIT
#------------------------------------------------------------------------------#

# Get the compiler defines that were used to build the library
# to pass to the flecsit script
get_directory_property(_defines DIRECTORY ${CMAKE_SOURCE_DIR}
  COMPILE_DEFINITIONS)
get_directory_property(_includes DIRECTORY ${CMAKE_SOURCE_DIR}
  INCLUDE_DIRECTORIES)

# Create string of compiler definitions for script
set(FLECSIT_COMPILE_DEFINES)
foreach(def ${_defines})
  set(FLECSIT_COMPILE_DEFINES
    "${FLECSIT_COMPILE_DEFINES} -D${def}")
endforeach()

string(STRIP "${FLECSIT_COMPILE_DEFINES}" FLECSIT_COMPILE_DEFINES)

# Create string of include directories for script
set(FLECSIT_INCLUDE_DIRECTORIES)
foreach(inc ${_includes})
  set(FLECSIT_INCLUDE_DIRECTORIES
    "${FLECSIT_INCLUDE_DIRECTORIES} -I${inc}")
endforeach()

string(STRIP "${FLECSIT_INCLUDE_DIRECTORIES}" FLECSIT_INCLUDE_DIRECTORIES)

# Create string of runtime link libraries for script
# Create list of link directories for LD_LIBRARY_PATH hint
set(FLECSIT_RUNTIME_LIBRARIES)
set(FLECSIT_LD_LIBRARY_PATH)
foreach(lib ${FLECSI_RUNTIME_LIBRARIES})
  # Runtime link libraries
  set(FLECSIT_RUNTIME_LIBRARIES
    "${FLECSIT_RUNTIME_LIBRARIES} ${lib}")

  # LD_LIBRARY_PATH hint
  get_filename_component(_path ${lib} DIRECTORY)
  list(APPEND FLECSIT_LD_LIBRARY_PATH ${_path})
endforeach()

string(STRIP "${FLECSI_RUNTIME_LIBRARIES}" FLECSI_RUNTIME_LIBRARIES)

# Append local build and remove duplicates
list(APPEND FLECSIT_LD_LIBRARY_PATH ${CMAKE_BINARY_DIR}/lib)
list(REMOVE_DUPLICATES FLECSIT_LD_LIBRARY_PATH)

string(STRIP "${FLECSIT_LD_LIBRARY_PATH}" FLECSIT_LD_LIBRARY_PATH)

#------------------------------------------------------------------------------#
# FleCSIT
#------------------------------------------------------------------------------#

option(ENABLE_FLECSIT "Enable FleCSIT Command-Line Tool" OFF)
set(FLECSI_PYTHON_PATH_MODULE)
set(FLECSI_PYTHON_PATH_BASH)
set(FLECSI_PYTHON_PATH_CSH)

if(ENABLE_FLECSIT)

	find_package(PythonInterp 2.7 REQUIRED)

	execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig as cg; print cg.get_python_lib(0,0,prefix='${CMAKE_INSTALL_PREFIX}')" OUTPUT_VARIABLE PYTHON_INSTDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

	install(DIRECTORY ${CMAKE_SOURCE_DIR}/tools/flecsit/flecsit
		DESTINATION ${PYTHON_INSTDIR}
		FILES_MATCHING PATTERN "*.py")

	configure_file(${CMAKE_SOURCE_DIR}/tools/flecsit/bin/flecsit.in
		${CMAKE_BINARY_DIR}/flecsit/bin/flecsit @ONLY)

	install(PROGRAMS ${CMAKE_BINARY_DIR}/flecsit/bin/flecsit
		DESTINATION bin
		PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE
			WORLD_READ WORLD_EXECUTE
	)

  set(FLECSI_PYTHON_PATH_MODULE "prepend-path PYTHONPATH ${PYTHON_INSTDIR}")
  set(FLECSI_PYTHON_PATH_BASH
    "export PYTHONPATH=\${PYTHONPATH}:${PYTHON_INSTDIR}")
  set(FLECSI_PYTHON_PATH_CSH
    "setenv PYTHONPATH $PYTHONPATH:${PYTHON_INSTDIR}")

endif()

#------------------------------------------------------------------------------#
# FleCSI environment module
#------------------------------------------------------------------------------#

configure_file(${CMAKE_SOURCE_DIR}/bin/flecsi.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi
  DESTINATION bin
  )

#------------------------------------------------------------------------------#
# Handle script and source files for FleCSIT tool
#------------------------------------------------------------------------------#

# Copy the auxiliary files for local development
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/share/runtime_main.cc
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/flecsi/execution/runtime_main.cc
    ${CMAKE_BINARY_DIR}/share/runtime_main.cc
    DEPENDS ${CMAKE_SOURCE_DIR}/flecsi/execution/runtime_main.cc
    COMMENT "Copying runtime main file")
add_custom_target(runtime_main ALL
  DEPENDS ${CMAKE_BINARY_DIR}/share/runtime_main.cc)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/share/runtime_driver.cc
  COMMAND ${CMAKE_COMMAND} -E copy
    ${_runtime_path}/runtime_driver.cc
    ${CMAKE_BINARY_DIR}/share/runtime_driver.cc
    DEPENDS ${_runtime_path}/runtime_driver.cc
    COMMENT "Copying runtime driver file")
add_custom_target(runtime_driver ALL
  DEPENDS ${CMAKE_BINARY_DIR}/share/runtime_driver.cc)

# Install the auxiliary files
install(FILES ${CMAKE_SOURCE_DIR}/flecsi/execution/runtime_main.cc
  DESTINATION share/flecsi/runtime)
install(FILES ${_runtime_path}/runtime_driver.cc
  DESTINATION share/flecsi/runtime)

#------------------------------------------------------------------------------#
# Helper shell environment setup
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.sh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.csh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh @ONLY)


# Install shell helpers
install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh
  DESTINATION bin
  RENAME flecsi.sh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh
  DESTINATION bin
  RENAME flecsi.csh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

#------------------------------------------------------------------------------#
# Static container
#------------------------------------------------------------------------------#

option(ENABLE_STATIC_CONTAINER "Enable static meta container" OFF)

set (MAX_CONTAINER_SIZE 6 CACHE INTEGER  "Set the depth of the container")
add_definitions( -DMAX_COUNTER_SIZE=${MAX_CONTAINER_SIZE} )

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
