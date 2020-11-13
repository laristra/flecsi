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
  annotation.hh
  array_ref.hh
  bit_buffer.hh
  bitutils.hh
  color_map.hh
  common.hh
  constant.hh
  dag.hh debruijn.hh
  demangle.hh
  dimensioned_array.hh
  export_definitions.hh
  function_traits.hh
  geometry/filling_curve.hh
  geometry/point.hh
  dcrs.hh
  parmetis.hh
  graphviz.hh
  hashtable.hh
  mpi.hh
  offset.hh
  reorder.hh
  serialize.hh
  set_intersection.hh
  set_utils.hh
  static_verify.hh
  target.hh
  tuple_visitor.hh
  tuple_walker.hh
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
# color_map
#------------------------------------------------------------------------------#

add_unit(color_map
  SOURCES
    test/color_map.cc
  THREADS 3
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
# serialize
#------------------------------------------------------------------------------#

add_unit(serialize
  SOURCES
    test/serialize.cc
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
# geometry
#------------------------------------------------------------------------------#

add_unit(point
  SOURCES geometry/test/point.cc
)

add_unit(filling_curve
  SOURCES geometry/test/filling_curve.cc
)

#------------------------------------------------------------------------------#
# hashtable
#------------------------------------------------------------------------------#

add_unit(hashtable
  SOURCES
    test/hashtable.cc
)

#------------------------------------------------------------------------------#
# annotation
#------------------------------------------------------------------------------#
if (NOT FLECSI_CALIPER_DETAIL STREQUAL "none")
add_unit(annotation
  SOURCES
  test/annotation.cc
  THREADS
  3 4
)
endif()
