#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Options
#------------------------------------------------------------------------------#

set(Legion_ROOT "" CACHE PATH "Root directory of Legion installation")

#------------------------------------------------------------------------------#
# Find the header file
#------------------------------------------------------------------------------#

find_path(Legion_INCLUDE_DIR legion.h
    HINTS ENV Legion_ROOT
    PATHS ${Legion_ROOT}
    PATH_SUFFIXES include)

if(Legion_INCLUDE_DIR)
  set(Legion_INCLUDE_DIRS
    ${Legion_INCLUDE_DIR}
    ${Legion_INCLUDE_DIR}/legion
    ${Legion_INCLUDE_DIR}/realm
    ${Legion_INCLUDE_DIR}/mappers)
endif()

#------------------------------------------------------------------------------#
# Find the legion and realm libraries
#------------------------------------------------------------------------------#

find_library(Legion_LIBRARY
    NAMES legion
    PATHS ${Legion_ROOT}
    PATH_SUFFIXES lib lib64)

find_library(REALM_LIBRARY
    NAMES realm
    PATHS ${Legion_ROOT}
    PATH_SUFFIXES lib lib64)

if(Legion_LIBRARY AND REALM_LIBRARY)
    list(APPEND Legion_LIBRARIES ${Legion_LIBRARY} ${REALM_LIBRARY})
endif()

#------------------------------------------------------------------------------#
# Set standard args stuff
#------------------------------------------------------------------------------#

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Legion
    REQUIRED_VARS
        Legion_INCLUDE_DIRS
        Legion_LIBRARIES)

mark_as_advanced(Legion_INCLUDE_DIR Legion_INCLUDE_DIRS Legion_LIBRARY
    Legion_ROOT REALM_LIBRARY)

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
