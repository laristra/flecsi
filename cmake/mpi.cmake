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

option(ENABLE_MPI "Enable MPI" OFF)
option(ENABLE_MPI_CXX_BINDINGS "Enable MPI C++ Bindings" OFF)
option(ENABLE_MPI_THREAD_MULITPLE "Enable MPI_THREAD_MULTIPLE" OFF)
mark_as_advanced(ENABLE_MPI_THREAD_MULITPLE)

mark_as_advanced(ENABLE_MPI_CXX_BINDINGS)

if(ENABLE_MPI)
  if(ENABLE_MPI_CXX_BINDINGS)
    # These are removed as of MPI-3 but are preserved for legacy cinch behavior
    find_package(MPI COMPONENTS C MPICXX REQUIRED)
    # Setting expected variable for rest of cinch
    set(MPI_LANGUAGE CXX)
  else()
    find_package(MPI COMPONENTS C CXX REQUIRED)
    # These almost definitely don't need to be set but are preserved for legacy cinch behavior
    #   Also, regardless, they should probably be added to a target and not used as a global
    #   definition to avoid contaminating other build systems
    add_definitions(-DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX)
    # Setting expected variable for rest of cinch
    set(MPI_LANGUAGE C)
  endif()
  # And append libraries, along with required flags and preprocessor defs, to expected variable
  list(APPEND FLECSI_RUNTIME_LIBRARIES MPI::MPI_CXX MPI::MPI_C)
endif()
