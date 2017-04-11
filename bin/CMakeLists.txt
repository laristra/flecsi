#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add source files for example application.
#------------------------------------------------------------------------------#

set(example_src example_app.cc example_driver.cc)
set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")
    list(APPEND example_src ${exec_path}/serial/runtime_driver.cc)
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")
    list(APPEND example_src ${exec_path}/legion/runtime_driver.cc
                            ${exec_path}/legion/context_policy.cc
                            ${exec_path}/../utils/debruijn.cc
)
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
    list(APPEND example_src ${exec_path}/mpilegion/runtime_driver.cc
                            ${exec_path}/mpilegion/copy_ghosts.cc
                            ${exec_path}/test/mpilegion/init_partitions_task.cc
                            ${exec_path}/../data/data_client.cc
                            ${exec_path}/../data/legion/dpd.cc
                            ${exec_path}/../utils/debruijn.cc
)
# FIXME: Remove this after refactor
elseif(FLECSI_RUNTIME_MODEL STREQUAL "rf_mpilegion")
    list(APPEND example_src ${exec_path}/rf_mpilegion/runtime_driver.cc)
else()
    message(FATAL_ERROR "This runtime is not yet supported")
endif()

#------------------------------------------------------------------------------#
# Add a rule to build the executable.
#------------------------------------------------------------------------------#

add_executable(example_app ${example_src})

#------------------------------------------------------------------------------#
# Add link dependencies.
#------------------------------------------------------------------------------#

target_link_libraries(example_app  ${FLECSI_RUNTIME_LIBRARIES})

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
