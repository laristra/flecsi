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

set(util_HEADERS
  array_ref.hh
  bit_buffer.hh
  bitutils.hh
  common.hh
  const_string.hh
  constant.hh
  crs.hh
  dag.hh debruijn.hh
  demangle.hh
  dimensioned_array.hh
  export_definitions.hh
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
  tuple_type_converter.hh
  tuple_visitor.hh
  tuple_walker.hh
  typeify.hh
  type_traits.hh
  uint128.hh
  unit.hh
  unit/output.hh
  unit/types.hh
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(util_SOURCES
  debruijn.cc
  demangle.cc
)

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(util_HEADERS
  ${util_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(util_SOURCES
  ${util_SOURCES}
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

add_unit(array_ref
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

add_unit(common
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

add_unit(crs
  SOURCES
    test/crs.cc
)

endif()
    

#------------------------------------------------------------------------------#
# demangle
#------------------------------------------------------------------------------#

add_unit(demangle
  SOURCES
    test/demangle.cc
)

#------------------------------------------------------------------------------#
# unit
#------------------------------------------------------------------------------#

add_unit(unit
  SOURCES
    test/unit.cc
  THREADS
    1 2 4
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  add_unit(unit_task
    SOURCES
      test/unit_task.cc
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

add_unit(function_traits
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

add_unit(id
  SOURCES
    test/id.cc
  INPUTS
    test/id.blessed
    ${id_blessed_input}
)

#------------------------------------------------------------------------------#
# hash
#------------------------------------------------------------------------------#

add_unit(hash
  SOURCES
    test/hash.cc
)

#------------------------------------------------------------------------------#
# serialize
#------------------------------------------------------------------------------#

add_unit(serialize
  SOURCES
    test/serialize.cc
)

#------------------------------------------------------------------------------#
# set_intersection
#------------------------------------------------------------------------------#

add_unit(set_intersection
  SOURCES
    test/set_intersection.cc
)

#------------------------------------------------------------------------------#
# set_utils
#------------------------------------------------------------------------------#

add_unit(set_utils
  SOURCES
    test/set_utils.cc
  INPUTS
    test/set_utils.blessed
)

#------------------------------------------------------------------------------#
# simple_id
#------------------------------------------------------------------------------#

add_unit(simple_id
  SOURCES
    test/simple_id.cc
)

#------------------------------------------------------------------------------#
# static_verify
#------------------------------------------------------------------------------#

add_unit(static_verify
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

add_unit(tuple_type_converter
  SOURCES
    test/tuple_type_converter.cc
  INPUTS
    ${tuple_type_converter_blessed_input}
)

#------------------------------------------------------------------------------#
# geometry
#------------------------------------------------------------------------------#

add_unit(point
  SOURCES geometry/test/point.cc
)

add_unit(filling_curve
  SOURCES geometry/test/filling_curve.cc
)
