#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add definitions.
#------------------------------------------------------------------------------#
#  add_definitions(-DFLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER)
  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_DRIVER)
  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT)

#------------------------------------------------------------------------------#
# Add source files for example application.
#------------------------------------------------------------------------------#

set(simple_drivers_src simple_drivers.cc )
set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution )

if(FLECSI_RUNTIME_MODEL STREQUAL "serial" )
    list(APPEND simple_drivers_src ${exec_path}/serial/runtime_driver.cc )
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")
    list(APPEND simple_drivers_src ${exec_path}/legion/runtime_driver.cc )
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
    list(APPEND simple_drivers_src ${exec_path}/mpi/runtime_driver.cc )
elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
    list(APPEND simple_drivers_src ${exec_path}/hpx/runtime_driver.cc )
else()
  message(FATAL_ERROR "Unrecognized runtime selection: ${FLECSI_RUNTIME_MODEL}")
endif()

#------------------------------------------------------------------------------#
# Add a rule to build the executable.
#------------------------------------------------------------------------------#

add_executable(simple_drivers ${simple_drivers_src})

#------------------------------------------------------------------------------#
# Add link dependencies.
#------------------------------------------------------------------------------#

target_link_libraries(simple_drivers FleCSI ${FLECSI_RUNTIME_LIBRARIES})

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
  hpx_setup_target(simple_drivers FOLDER "Examples")
else()
  set_target_properties(simple_drivers PROPERTIES FOLDER "Examples")
endif()

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
