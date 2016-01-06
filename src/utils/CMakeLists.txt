#~----------------------------------------------------------------------------~#
#  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
# /@@///// /@@      /@@///// //@@   @@ /@@
# /@@      /@@      /@@       //@@ @@  /@@
# /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
# /@@////  /@@      /@@////     @@/@@  /@@
# /@@      /@@      /@@        @@ //@@ /@@
# /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
# //       //////// //////// //     // // 
# 
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#~----------------------------------------------------------------------------~#

set(utils_HEADERS
  dummy.h
  check_types.h
  static_for.h     detail/static_for.h
  tuple_for_each.h detail/tuple_for_each.h
  tuple_visit.h    detail/tuple_visit.h
  tuple_zip.h      detail/tuple_zip.h
  zip.h            detail/zip.h
  PARENT_SCOPE)

set(utils_SOURCES
  dummy.cc
  PARENT_SCOPE)

cinch_add_unit(bitfield
  SOURCES test/bitfield.cc)

cinch_add_unit(const_string
  SOURCES test/const_string.cc)

cinch_add_unit(factory
  SOURCES test/factory.cc)

cinch_add_unit(zip
  SOURCES test/zip.cc)

cinch_add_unit(static_for
  SOURCES test/static_for.cc)

cinch_add_unit(tuple_for_each
  SOURCES test/tuple_for_each.cc)

cinch_add_unit(tuple_visit
  SOURCES test/tuple_visit.cc)

cinch_add_unit(tuple_zip
  SOURCES test/tuple_zip.cc)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
