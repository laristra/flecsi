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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  common/function_handle.h
  common/launch.h
  common/processor.h
  context.h
  execution.h
  global_object_wrapper.h
  task.h
)

set(execution_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/internal_task.h
    legion/mapper.h
    legion/runtime.h
    legion/task_wrapper.h
    legion/tasks.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/context_policy.h
    mpi/execution_policy.h
    mpi/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
    mpi/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
    hpx/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    hpx/runtime.cc
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(execution_SOURCES
  ${execution_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Executables.
#------------------------------------------------------------------------------#

add_executable(sanity
  test/sanity.cc
  legion/context_policy.cc
  ../utils/flog/packet.cc
  ${CMAKE_SOURCE_DIR}/cinch/cinch/runtime.cc
)

include_directories(${CMAKE_SOURCE_DIR})

target_compile_definitions(sanity PUBLIC -DCINCH_RUNTIME_DEBUG)

target_link_libraries(sanity
  ${CINCH_RUNTIME_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

ftest_add_unit(color
  SOURCES
    test/color.cc
  THREADS
    4
)

ftest_add_unit(global-object
  SOURCES
    test/global-object.cc
)
