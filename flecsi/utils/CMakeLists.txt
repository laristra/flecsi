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
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_HEADERS
  array_ref.hh
  bit_buffer.hh
  bitutils.hh
  checksum.hh
  common.hh
  const_string.hh
  crs.hh
  dag.hh debruijn.hh
  demangle.hh
  dimensioned_array.hh
  export_definitions.hh
  factory.hh
  ftest.hh
  ftest/node.hh
  ftest/output.hh
  ftest/types.hh
  function_traits.hh
  geometry/filling_curve.hh
  geometry/point.hh
  graph/colorer.hh
  graph/parmetis_colorer.hh
  graphviz.hh
  hash.hh
  id.hh
  macros.hh
  mpi_type_traits.hh
  offset.hh
  reorder.hh
  serialize.hh
  set_intersection.hh
  set_utils.hh
  simple_id.hh
  static_verify.hh
  target.hh
  tuple_function.hh
  tuple_type_converter.hh
  tuple_visitor.hh
  tuple_walker.hh
  tuple_wrapper.hh
  typeify.hh
  type_traits.hh
  uint128.hh
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
# common
#------------------------------------------------------------------------------#

set(common_blessed_input test/common.blessed.gnug)

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
# crs
#------------------------------------------------------------------------------#

if(FLECSI_ENABLE_PARMETIS)

ftest_add_unit(crs
  SOURCES
    test/crs.cc
)

endif()
    

#------------------------------------------------------------------------------#
# demangle
#------------------------------------------------------------------------------#

ftest_add_unit(demangle
  SOURCES
    test/demangle.cc
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

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  ftest_add_unit(ftest_task
    SOURCES
      test/ftest_task.cc
    THREADS 2
  )
endif()

#------------------------------------------------------------------------------#
# function_traits
#------------------------------------------------------------------------------#

set(function_traits_input test/function_traits.blessed.gnug)

if(MSVC)
  set(function_traits_input test/function_traits.blessed.msvc)
endif()

ftest_add_unit(function_traits
  SOURCES
    test/function_traits.cc
  INPUTS
    test/function_traits.blessed
    ${function_traits_input}
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
# serialize
#------------------------------------------------------------------------------#

ftest_add_unit(serialize
  SOURCES
    test/serialize.cc
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
# tuple_type_converter
#------------------------------------------------------------------------------#

set(tuple_type_converter_blessed_input test/tuple_type_converter.blessed.gnug)

if(MSVC)
  set(tuple_type_converter_blessed_input
    test/tuple_type_converter.blessed.msvc)
endif()

ftest_add_unit(tuple_type_converter
  SOURCES
    test/tuple_type_converter.cc
  INPUTS
    ${tuple_type_converter_blessed_input}
)

#------------------------------------------------------------------------------#
# geometry
#------------------------------------------------------------------------------#

ftest_add_unit(point
  SOURCES geometry/test/point.cc
)

ftest_add_unit(filling_curve
  SOURCES geometry/test/filling_curve.cc
)
