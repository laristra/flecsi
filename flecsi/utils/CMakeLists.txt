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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_HEADERS
  array_ref.h
  annotation.h
  bit_buffer.h
  checksum.h
  common.h
  const_string.h
  dag.h
  debruijn.h
  demangle.h
  dimensioned_array.h
  export_definitions.h
  factory.h
  fixed_vector.h
  function_traits.h
  graphviz.h
  hash.h
  humble.h
  id.h
  logging.h
  macros.h
  mpi_type_traits.h
  offset.h
  reorder.h
  serialize.h
  set_intersection.h
  set_utils.h
  simple_id.h
  static_verify.h
  target.h
  tuple_type_converter.h
  tuple_visit.h
  tuple_walker.h
  tuple_wrapper.h
  type_traits.h
  typeify.h
  utility.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_SOURCES
  debruijn.cc
  demangle.cc
)

#------------------------------------------------------------------------------#
# Runtime-specific files for unit tests.
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

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER ../execution/hpx/runtime_driver.cc)

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
  LIBRARIES
    ${CINCH_RUNTIME_LIBRARIES}
  POLICY ${UNIT_POLICY}
)

set(factory_blessed_input test/factory.blessed.gnug)

if(MSVC)
  set(factory_blessed_input test/factory.blessed.msvc)
endif()

cinch_add_unit(factory
  SOURCES
    test/factory.cc
  INPUTS
    test/factory.blessed
    ${factory_blessed_input}
  LIBRARIES
    FleCSI
)

cinch_add_unit(fixed_vector
  SOURCES
    test/fixed_vector.cc
)


cinch_add_unit(reorder
  SOURCES
    test/reorder.cc
)

cinch_add_unit(humble
  SOURCES
    test/humble.cc
)

cinch_add_unit(logging
  SOURCES
    test/logging.cc
)

cinch_add_unit(serialize
  SOURCES
    test/serialize.cc
)

cinch_add_unit(set_intersection
  SOURCES
    test/set_intersection.cc
)

cinch_add_unit(set_utils
  SOURCES
    test/set_utils.cc
  INPUTS
    test/set_utils.blessed
)

cinch_add_unit(simple_id
  SOURCES
    test/simple_id.cc
)

cinch_add_unit(static_verify
  SOURCES
    test/static_verify.cc
)

cinch_add_unit(test_utility
  SOURCES
    test/utility.cc
  INPUTS
    test/utility.blessed.gnug
  LIBRARIES
    FleCSI
)

set(tuple_type_converter_blessed_input test/tuple_type_converter.blessed.gnug)

if(MSVC)
  set(tuple_type_converter_blessed_input test/tuple_type_converter.blessed.msvc)
endif()

cinch_add_unit(tuple_type_converter
  SOURCES
    test/tuple_type_converter.cc
  INPUTS
    ${tuple_type_converter_blessed_input}
  LIBRARIES
    FleCSI
)

cinch_add_unit(tuple_wrapper
  SOURCES
    test/tuple_wrapper.cc
  LIBRARIES
    FleCSI
)

set(array_ref_blessed_input test/array_ref.blessed.gnug)

if(MSVC)
  set(array_ref_blessed_input test/array_ref.blessed.msvc)
endif()

cinch_add_unit(array_ref
  SOURCES
    test/array_ref.cc
  INPUTS
    test/array_ref.blessed ${array_ref_blessed_input}
  LIBRARIES
    FleCSI
)

set(common_blessed_input test/common.blessed.gnug)

if(MSVC)
  set(common_blessed_input test/common.blessed.msvc)
endif()

cinch_add_unit(common
  SOURCES
    test/common.cc
  INPUTS
    test/common.blessed.ppc test/common.blessed ${common_blessed_input}
  LIBRARIES
    FleCSI
)

set(id_blessed_input test/id.blessed.gnug)

if(MSVC)
  set(id_blessed_input test/id.blessed.msvc)
endif()

cinch_add_unit(id
  SOURCES
    test/id.cc
  INPUTS
    test/id.blessed ${id_blessed_input}
  LIBRARIES
    FleCSI
)

if(ENABLE_OPENSSL)
  cinch_add_unit(checksum
    SOURCES
      test/checksum.cc
    LIBRARIES
      ${OPENSSL_LIBRARIES}
  )
endif()

cinch_add_unit(debruijn
  SOURCES
    debruijn.cc
    test/debruijn.cc
)

cinch_add_unit(tuple_walker
  SOURCES
    test/tuple_walker.cc
  INPUTS
    test/tuple_walker.blessed
)
