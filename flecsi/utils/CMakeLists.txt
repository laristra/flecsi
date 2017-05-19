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
  dbc.h
  dbc_impl.h
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
  dbc_impl.cc
  common.cc
  PARENT_SCOPE
)

set(utils_HEADERS ${utils_HEADERS} PARENT_SCOPE)

mcinch_add_unit(any
  SOURCES test/any.cc
)

mcinch_add_unit(dbc
  SOURCES test/dbc_test.cc
  LIBRARIES flecsi
)

mcinch_add_unit(dbc-no-assert
  SOURCES test/dbc_test_no_assert.cc
  LIBRARIES flecsi
)

mcinch_add_unit(dbc-notify
  SOURCES test/dbc_test_notify.cc
  LIBRARIES flecsi
)

mcinch_add_unit(const_string
  SOURCES test/const_string.cc
)

mcinch_add_unit(factory
  SOURCES test/factory.cc
)

mcinch_add_unit(reorder
  SOURCES test/reorder.cc
)

mcinch_add_unit(reflection
  SOURCES
    test/reflection.cc
  POLICY
    SERIAL_DEVEL
)

if(ENABLE_OPENSSL)
  mcinch_add_unit(checksum
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
