#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from string import Template

cmake_source_template = Template(
"""
#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

set(${PARENT}_HEADERS
${HEADERS}${SPACES}PARENT_SCOPE
)

set(${PARENT}_SOURCES
${SOURCES}${SPACES}PARENT_SCOPE
)

#if(ENABLE_UNIT_TESTS)
#${SPACES}cinch_add_unit(casename
#${SPACES}${SPACES}SOURCES testfile.cc
#${SPACES}${SPACES}LIBRARIES list
#${SPACES}${SPACES}INCLUDES list
#${SPACES}${SPACES}POLICY MPI
#${SPACES}${SPACES}THREADS 1 2 4
#${SPACES})
#endif(ENABLE_UNIT_TESTS)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=${TABSTOP} shiftwidth=${TABSTOP} expandtab :
#----------------------------------------------------------------------------~-#
""")

cmake_app_template = Template(
"""
#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add a rule to build the executable
#------------------------------------------------------------------------------#

add_executable(executable file1.cc)

#------------------------------------------------------------------------------#
# Add link dependencies
#------------------------------------------------------------------------------#

target_link_libraries(executable libraries)

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
""")
