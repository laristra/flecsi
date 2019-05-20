
project(simple_mesh_2d)

# We need C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

cmake_minimum_required(VERSION 3.11.0)

# We are going to use this app with MPI (for now - later we can add Legion)
#find_package(MPI REQUIRED)
#set(CMAKE_C_COMPILER ${MPI_C_COMPILER} CACHE FILEPATH "C compiler to use" FORC
#E)
#set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER} CACHE FILEPATH "C++ compiler to use
#" FORCE)

#Installation directory
if (NOT INSTALL_DIR)
  set (INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif ()

# FleCSI is required to build this standalone app
if (FLECSI_DIR)
  find_package(FleCSI REQUIRED
               HINTS ${FLECSI_DIR}/lib64/cmake ${FLECSI_DIR}/lib/cmake)

  include_directories(${FLECSI_INCLUDE_DIRS})

  message(STATUS "FLECSI_INCLUDE_DIRS ${FLECSI_INCLUDE_DIRS}")
else()
  message(FATAL "FLECSI_DIR must be defined")
endif(FLECSI_DIR)


#simple_mesh_2d 
add_executable(simple_mesh_2d ${PROJECT_SOURCE_DIR}/simple_mesh_2d.cc ${PROJECT_SOURCE_DIR}/coloring/add_colorings.cc  ${FLECSI_RUNTIME_DRIVER} ${FLECSI_RUNTIME_MAIN})
target_link_libraries(simple_mesh_2d ${FleCSI_LIBRARIES} ${FLECSI_LIBRARY_DEPENDENCIES} ${FLECSI_RUNTIME_LIBRARIES})
target_compile_definitions(simple_mesh_2d PRIVATE FLECSI_ENABLE_SPECIALIZATION_TLT_INIT FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input/simple2d-16x16.msh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
