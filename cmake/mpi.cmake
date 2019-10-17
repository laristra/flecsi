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

#------------------------------------------------------------------------------#
# Skip C++ linkage of MPI
#------------------------------------------------------------------------------#

if(ENABLE_MPI_CXX_BINDINGS)
    find_package(MPI REQUIRED COMPONENTS CXX)
    set(MPI_LANGUAGE CXX)
else()
    find_package(MPI REQUIRED COMPONENTS C)
    set(MPI_LANGUAGE C)
    # Globally add these compile definitions for linking C++ applications
    # with the C-version of MPI.  Might be a better way to do this.
    add_definitions(-DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX)
endif(ENABLE_MPI_CXX_BINDINGS)

if(MPI_${MPI_LANGUAGE}_FOUND)
    include_directories(SYSTEM ${MPI_${MPI_LANGUAGE}_INCLUDE_PATH})

    # using mpich, there are extra spaces that cause some issues
    separate_arguments(MPI_${MPI_LANGUAGE}_COMPILE_FLAGS)

    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${MPI_${MPI_LANGUAGE}_LIBRARIES})
endif(MPI_${MPI_LANGUAGE}_FOUND)

if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    add_definitions(-D_HAS_AUTO_PTR_ETC=1)
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    add_definitions(-DGTEST_LANG_CXX11=1)
endif()

endif(ENABLE_MPI)
