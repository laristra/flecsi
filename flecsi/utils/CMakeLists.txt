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

################################################################################
# Unit tests.
################################################################################

#------------------------------------------------------------------------------#
# array_ref
#------------------------------------------------------------------------------#

set(array_ref_blessed_input test/array_ref.blessed.gnug)

if(MSVC)
  set(array_ref_blessed_input test/array_ref.blessed.msvc)
endif()

ftest_add_unit(array_ref
  SOURCES
    test/array_ref.cc
  INPUTS
    test/array_ref.blessed
    ${array_ref_blessed_input}
)

#------------------------------------------------------------------------------#
# array_ref
#------------------------------------------------------------------------------#

#set(common_blessed_input test/common.blessed.gnug)

if(MSVC)
  set(common_blessed_input test/common.blessed.msvc)
endif()

ftest_add_unit(common
  SOURCES
    test/common.cc
  INPUTS
    test/common.blessed.ppc
    test/common.blessed
    ${common_blessed_input}
)

#------------------------------------------------------------------------------#
# factory
#------------------------------------------------------------------------------#

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

#------------------------------------------------------------------------------#
# ftest
#------------------------------------------------------------------------------#

ftest_add_unit(ftest
  SOURCES
    test/ftest.cc
  THREADS
    1 2 4
)

#------------------------------------------------------------------------------#
# id
#------------------------------------------------------------------------------#

set(id_blessed_input test/id.blessed.gnug)

if(MSVC)
  set(id_blessed_input test/id.blessed.msvc)
endif()

ftest_add_unit(id
  SOURCES
    test/id.cc
  INPUTS
    test/id.blessed
    ${id_blessed_input}
)

#------------------------------------------------------------------------------#
# hash
#------------------------------------------------------------------------------#

ftest_add_unit(hash
  SOURCES
    test/hash.cc
)

#------------------------------------------------------------------------------#
# reorder
#------------------------------------------------------------------------------#

ftest_add_unit(reorder
  SOURCES
		test/reorder.cc
)

#------------------------------------------------------------------------------#
# set_intersection
#------------------------------------------------------------------------------#

ftest_add_unit(set_intersection
  SOURCES
    test/set_intersection.cc
)

#------------------------------------------------------------------------------#
# set_utils
#------------------------------------------------------------------------------#

ftest_add_unit(set_utils
  SOURCES
    test/set_utils.cc
  INPUTS
    test/set_utils.blessed
)

#------------------------------------------------------------------------------#
# simple_id
#------------------------------------------------------------------------------#

ftest_add_unit(simple_id
  SOURCES
    test/simple_id.cc
)

#------------------------------------------------------------------------------#
# static_verify
#------------------------------------------------------------------------------#

ftest_add_unit(static_verify
  SOURCES
    test/static_verify.cc
)

#------------------------------------------------------------------------------#
# utils_iterator
#------------------------------------------------------------------------------#

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
