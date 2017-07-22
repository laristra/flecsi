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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_HEADERS
  any.h
  array_ref.h
  dbc.h
  dbc_impl.h
  checksum.h
  common.h
  const_string.h
  debruijn.h
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
  tuple_type_converter.h
  tuple_walker.h
  tuple_wrapper.h
  type_traits.h
  utility.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_SOURCES
  dbc_impl.cc
  common.cc
  debruijn.cc
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(utils_HEADERS
  ${utils_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(utils_SOURCES
  ${utils_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_devel_target(clog
  SOURCES
    test/clog.cc
  POLICY ${UNIT_POLICY}
)

# Broken by refactor
#cinch_add_unit(dbc
#  SOURCES test/dbc_test.cc
#  POLICY SERIAL
#)

# Broken by refactor
#cinch_add_unit(dbc-no-assert
#  SOURCES test/dbc_test_no_assert.cc
#  POLICY SERIAL
#)

# Broken by refactor
#cinch_add_unit(dbc-notify
#  SOURCES test/dbc_test_notify.cc
#  POLICY SERIAL
#)

cinch_add_unit(const_string
  SOURCES test/const_string.cc
)

cinch_add_unit(factory
  SOURCES test/factory.cc
  INPUTS  test/factory.blessed
)

cinch_add_unit(reorder
  SOURCES test/reorder.cc
)

cinch_add_unit(any
  SOURCES test/any.cc
  INPUTS  test/any.blessed
)

cinch_add_unit(reflection
  SOURCES
    test/reflection.cc
  POLICY
    SERIAL_DEVEL
)

cinch_add_unit(hash
  SOURCES test/hash.cc
)

cinch_add_unit(humble
  SOURCES test/humble.cc
)

cinch_add_unit(utils_iterator
  SOURCES test/iterator.cc
  INPUTS  test/iterator.blessed
)

cinch_add_unit(logging
  SOURCES test/logging.cc
)

cinch_add_unit(set_intersection
  SOURCES test/set_intersection.cc
)

cinch_add_unit(set_utils
  SOURCES test/set_utils.cc
  INPUTS  test/set_utils.blessed
)

cinch_add_unit(static_verify
  SOURCES test/static_verify.cc
)

cinch_add_unit(test_utility
  SOURCES test/utility.cc
  INPUTS  test/utility.blessed
)

cinch_add_unit(tuple_type_converter
  SOURCES test/tuple_type_converter.cc
  INPUTS  test/tuple_type_converter.blessed
)

cinch_add_unit(tuple_walker
  SOURCES test/tuple_walker.cc
  INPUTS  test/tuple_walker.blessed
)

cinch_add_unit(tuple_wrapper
  SOURCES test/tuple_wrapper.cc
)

cinch_add_unit(tuple_function
  SOURCES test/tuple_function.cc
)

cinch_add_unit(index_space
  SOURCES test/index_space.cc
  INPUTS  test/index_space.blessed
)

cinch_add_unit(array_ref
  SOURCES test/array_ref.cc
  INPUTS  test/array_ref.blessed
)

cinch_add_unit(common
  SOURCES common.cc
          test/common.cc
  INPUTS  test/common.blessed
)

cinch_add_unit(debruijn
  SOURCES debruijn.cc
          test/debruijn.cc
)

cinch_add_unit(id
  SOURCES test/id.cc
  INPUTS  test/id.blessed
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
