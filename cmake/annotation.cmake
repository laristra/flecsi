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

set(FLECSI_CALIPER_DETAILS none low medium high)
if(NOT FLECSI_CALIPER_DETAIL)
  list(GET FLECSI_CALIPER_DETAILS 0 FLECSI_CALIPER_DETAIL)
endif()
set(FLECSI_CALIPER_DETAIL "${FLECSI_CALIPER_DETAIL}" CACHE STRING
  "Select the Caliper annotation detail (none,low,medium,high)")
set_property(CACHE FLECSI_CALIPER_DETAIL
  PROPERTY STRINGS ${FLECSI_CALIPER_DETAILS})


if (NOT FLECSI_CALIPER_DETAIL STREQUAL "none")
  find_package(caliper REQUIRED)

  message(STATUS "Found Caliper")

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES caliper)
endif()
