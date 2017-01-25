#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

set(utils_HEADERS
  any.h
  array_ref.h
  bitfield.h
  common.h
  const_string.h
  dimensioned_array.h
  factory.h
  hash.h
  humble.h
  id.h
  index_space.h
  iterator.h
  logging.h
  reflection.h
  reorder.h
  set_intersection.h
  set_utils.h
  static_verify.h
  tuple_function.h
  tuple_wrapper.h
  utility.h
)

set(utils_SOURCES
  common.cc
  PARENT_SCOPE
)

set(utils_HEADERS ${utils_HEADERS} PARENT_SCOPE)

cinch_add_unit(const_string
  SOURCES test/const_string.cc
)

cinch_add_unit(factory
  SOURCES test/factory.cc
)

cinch_add_unit(reorder
  SOURCES test/reorder.cc
)

cinch_add_unit(any
  SOURCES test/any.cc
)

cinch_add_unit(reflection
  SOURCES
    test/reflection.cc
  POLICY
    SERIAL_DEVEL
)

if(ENABLE_OPENSSL)
  cinch_add_unit(checksum
    SOURCES
      test/checksum.cc
    LIBRARIES
      ${OPENSSL_LIBRARIES}
  )
endif()

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
