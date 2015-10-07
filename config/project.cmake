#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

project(flexi)

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

cinch_add_application_directory("examples")

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(flexi src)

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

#------------------------------------------------------------------------------#
# Add build options
#------------------------------------------------------------------------------#

option(ENABLE_IO "Enable I/O with third party libraries." OFF)
if(ENABLE_IO)
  set(TPL_INSTALL_PREFIX /path/to/third/party/install CACHE PATH
  "/path/to/third/party/install")
  include_directories(${TPL_INSTALL_PREFIX}/include)
  set(IO_LIBRARIES ${TPL_INSTALL_PREFIX}/lib/libexodus.a
    ${TPL_INSTALL_PREFIX}/lib/libnetcdf.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5_hl.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5.a
    ${TPL_INSTALL_PREFIX}/lib/libszip.a
    ${TPL_INSTALL_PREFIX}/lib/libz.a
    -ldl)
endif(ENABLE_IO)
#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
