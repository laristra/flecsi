#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

project(flecsi)

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

cinch_add_application_directory("examples")

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(flecsi src)

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

#------------------------------------------------------------------------------#
# Add build options
#------------------------------------------------------------------------------#

set( TPL_INSTALL_PREFIX /path/to/third/party/install 
                        CACHE PATH
                        "path to thirdparty install" )
if (NOT TPL_INSTALL_PREFIX STREQUAL "")
  set(METIS_ROOT  ${TPL_INSTALL_PREFIX})
  set(SCOTCH_ROOT ${TPL_INSTALL_PREFIX})
endif()

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
