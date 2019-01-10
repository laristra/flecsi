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
  flog.h
  flog/message.h
  flog/packet.h
  flog/types.h
  flog/utils.h
  function_traits.h
  graphviz.h
  hash.h
  humble.h
  id.h
  index_space.h
  iterator.h
  logging.h
  macros.h
  mpi_type_traits.h
  offset.h
  reorder.h
  set_intersection.h
  set_utils.h
  simple_id.h
  static_verify.h
  tuple_function.h
  tuple_type_converter.h
  tuple_visit.h
  tuple_walker.h
  tuple_wrapper.h
  type_traits.h
  typeify.h
  uint128.h
  utility.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_SOURCES
  debruijn.cc
  demangle.cc
  flog/packet.cc
)

#------------------------------------------------------------------------------#
# Runtime-specific files for unit tests.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER ../execution/hpx/runtime.cc)

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

ftest_add_unit(ftest
  SOURCES
    test/ftest.cc
  THREADS
    4
)

ftest_add_unit(hash
  SOURCES
    test/hash.cc
)

ftest_add_unit(reorder
  SOURCES
		test/reorder.cc
)

set(factory_blessed_input test/factory.blessed.gnug)

if(MSVC)
  set(factory_blessed_input test/factory.blessed.msvc)
endif()

ftest_add_unit(factory
  SOURCES
    demangle.cc
    test/factory.cc
  INPUTS
    test/factory.blessed
    ${factory_blessed_input}
)

set(id_blessed_input test/id.blessed.gnug)

if(MSVC)
  set(id_blessed_input test/id.blessed.msvc)
endif()

ftest_add_unit(id
  SOURCES
    test/id.cc
  INPUTS
    test/id.blessed ${id_blessed_input}
)

set(iterator_blessed_input test/iterator.blessed.gnug)

if(MSVC)
  set(iterator_blessed_input test/iterator.blessed.msvc)
endif()

ftest_add_unit(utils_iterator
  SOURCES
    demangle.cc
    test/iterator.cc
  INPUTS
    test/iterator.blessed
    ${iterator_blessed_input}
)

ftest_add_unit(set_intersection
  SOURCES
    test/set_intersection.cc
)

ftest_add_unit(set_utils
  SOURCES
    test/set_utils.cc
  INPUTS
    test/set_utils.blessed
)

ftest_add_unit(simple_id
  SOURCES
    test/simple_id.cc
)

ftest_add_unit(static_verify
  SOURCES
    test/static_verify.cc
)
