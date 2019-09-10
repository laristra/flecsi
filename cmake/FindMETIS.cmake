#------------------------------------------------------------------------------#
# Copyright (c) 2016 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

# - Find metis
# Find the native METIS headers and libraries.
#
#  METIS_INCLUDE_DIRS - where to find metis.h, etc.
#  METIS_LIBRARIES    - List of libraries when using metis.
#  METIS_FOUND        - True if metis found.

#=============================================================
# _METIS_GET_VERSION
# Internal function to parse the version number in metis.h
#   _OUT_major = Major version number
#   _OUT_minor = Minor version number
#   _OUT_micro = Micro version number
#   _metisversion_hdr = Header file to parse
#=============================================================
function(_METIS_GET_VERSION _OUT_major _OUT_minor _OUT_micro _metisversion_hdr)
    file(STRINGS ${_metisversion_hdr} _contents REGEX "#define METIS_VER_[A-Z]+[ \t]+")
    if(_contents)
        string(REGEX REPLACE ".*#define METIS_VER_MAJOR[ \t]+([0-9]+).*" "\\1" ${_OUT_major} "${_contents}")
	string(REGEX REPLACE ".*#define METIS_VER_MINOR[ \t]+([0-9]+).*" "\\1" ${_OUT_minor} "${_contents}")
	string(REGEX REPLACE ".*#define METIS_VER_SUBMINOR[ \t]+([0-9]+).*" "\\1" ${_OUT_micro} "${_contents}")

        if(NOT ${_OUT_major} MATCHES "[0-9]+")
            message(FATAL_ERROR "Version parsing failed for METIS_VER_MAJOR!")
        endif()
        if(NOT ${_OUT_minor} MATCHES "[0-9]+")
            message(FATAL_ERROR "Version parsing failed for METIS_VER_MINOR!")
        endif()
        if(NOT ${_OUT_micro} MATCHES "[0-9]+")
            message(FATAL_ERROR "Version parsing failed for METIS_VER_SUBMINOR!")
        endif()

        set(${_OUT_major} ${${_OUT_major}} PARENT_SCOPE)
        set(${_OUT_minor} ${${_OUT_minor}} PARENT_SCOPE)
        set(${_OUT_micro} ${${_OUT_micro}} PARENT_SCOPE)

    elseif(EXISTS ${_metisversion_hdr})
        message(FATAL_ERROR "No METIS_VER_MINOR found in ${_metisversion_hdr} (metis too old")        
    else()        
	message(FATAL_ERROR "Include file ${_metisversion_hdr} does not exist")
    endif()
endfunction()

find_package(PkgConfig)

pkg_check_modules(PC_METIS metis)

# Look for the header file.
FIND_PATH(METIS_INCLUDE_DIR NAMES metis.h HINTS ${PC_METIS_INCLUDE_DIRS})
if(METIS_INCLUDE_DIR)
  _METIS_GET_VERSION(METIS_MAJOR_VERSION METIS_MINOR_VERSION METIS_PATCH_VERSION ${METIS_INCLUDE_DIR}/metis.h)
  set(METIS_VERSION ${METIS_MAJOR_VERSION}.${METIS_MINOR_VERSION}.${METIS_PATCH_VERSION})
else()
  set(METIS_VERSION 0.0.0)
endif()

# Look for the library.
FIND_LIBRARY(METIS_LIBRARY NAMES metis libmetis HINTS ${PC_METIS_LIBRARY_DIRS} )

# handle the QUIETLY and REQUIRED arguments and set METIS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(METIS REQUIRED_VARS METIS_LIBRARY METIS_INCLUDE_DIR VERSION_VAR METIS_VERSION)

# Copy the results to the output variables.
SET(METIS_LIBRARIES ${METIS_LIBRARY})
SET(METIS_INCLUDE_DIRS ${METIS_INCLUDE_DIR})

MARK_AS_ADVANCED(METIS_INCLUDE_DIR METIS_LIBRARY)
