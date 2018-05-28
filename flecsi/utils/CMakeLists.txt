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
  any.h
  array_ref.h
  bit_buffer.h
  checksum.h
  common.h
  const_string.h
  dag.h
  debruijn.h
  dimensioned_array.h
  export_definitions.h
  factory.h
  graphviz.h
  hash.h
  humble.h
  id.h
  index_space.h
  iterator.h
  logging.h
  offset.h
  reflection.h
  reorder.h
  set_intersection.h
  set_utils.h
  simple_id.h
  static_verify.h
  tuple_function.h
  tuple_type_converter.h
  tuple_walker.h
  tuple_wrapper.h
  type_traits.h
  uint128.h
  utility.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(utils_SOURCES
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
if(NOT FLECSI_RUNTIME_MODEL STREQUAL "hpx")
cinch_add_devel_target(clog
  SOURCES
    test/clog.cc
  POLICY ${UNIT_POLICY}
  FOLDER "Tests/Util"
)
endif()

cinch_add_unit(const_string
  SOURCES test/const_string.cc common.cc
  FOLDER "Tests/Util"
)

set(factory_blessed_input test/factory.blessed.gnug)
if(MSVC)
  set(factory_blessed_input test/factory.blessed.msvc)
endif()
cinch_add_unit(factory
  SOURCES test/factory.cc common.cc
  INPUTS  test/factory.blessed ${factory_blessed_input}
  FOLDER "Tests/Util"
)

cinch_add_unit(reorder
  SOURCES test/reorder.cc
  FOLDER "Tests/Util"
)

set(any_blessed_input test/any.blessed.gnug)
if(MSVC)
  set(any_blessed_input test/any.blessed.msvc)
endif()
cinch_add_unit(any
  SOURCES test/any.cc common.cc
  SOURCES test/any.cc
  INPUTS  test/any.blessed ${any_blessed_input}
  FOLDER "Tests/Util"
)

if(NOT MSVC)
  # FIXME: MSVC's preprocessor is too broken for this
  cinch_add_unit(reflection
  SOURCES
    test/reflection.cc
  POLICY
    SERIAL_DEVEL
    FOLDER
      "Tests/Util"
  )
endif()

cinch_add_unit(hash
  SOURCES test/hash.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(humble
  SOURCES test/humble.cc
  FOLDER "Tests/Util"
)

set(iterator_blessed_input test/iterator.blessed.gnug)
if(MSVC)
  set(iterator_blessed_input test/iterator.blessed.msvc)
endif()
cinch_add_unit(utils_iterator
  SOURCES test/iterator.cc common.cc
  INPUTS  test/iterator.blessed ${iterator_blessed_input}
  FOLDER "Tests/Util"
)

cinch_add_unit(logging
  SOURCES test/logging.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(set_intersection
  SOURCES test/set_intersection.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(set_utils
  SOURCES test/set_utils.cc
  INPUTS  test/set_utils.blessed
  FOLDER "Tests/Util"
)

cinch_add_unit(simple_id
  SOURCES test/simple_id.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(static_verify
  SOURCES test/static_verify.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(test_utility
  SOURCES test/utility.cc common.cc
  INPUTS  test/utility.blessed.gnug
  FOLDER "Tests/Util"
)

set(tuple_type_converter_blessed_input test/tuple_type_converter.blessed.gnug)
if(MSVC)
  set(tuple_type_converter_blessed_input test/tuple_type_converter.blessed.msvc)
endif()
cinch_add_unit(tuple_type_converter
  SOURCES test/tuple_type_converter.cc common.cc
  INPUTS  ${tuple_type_converter_blessed_input}
  FOLDER "Tests/Util"
)

cinch_add_unit(tuple_walker
  SOURCES test/tuple_walker.cc
  INPUTS  test/tuple_walker.blessed
  FOLDER "Tests/Util"
)

cinch_add_unit(tuple_wrapper
  SOURCES test/tuple_wrapper.cc common.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(tuple_function
  SOURCES test/tuple_function.cc
  FOLDER "Tests/Util"
)

cinch_add_unit(dag
  SOURCES test/dag.cc
  LIBRARIES ${FLECSI_LIBRARY_DEPENDENCIES}
  FOLDER "Tests/Util"
)

set(index_space_blessed_input test/index_space.blessed.gnug)
if(MSVC)
  set(index_space_blessed_input test/index_space.blessed.msvc)
endif()
cinch_add_unit(index_space
  SOURCES test/index_space.cc common.cc
  INPUTS  test/index_space.blessed ${index_space_blessed_input}
  FOLDER "Tests/Util"
)

set(array_ref_blessed_input test/array_ref.blessed.gnug)
if(MSVC)
  set(array_ref_blessed_input test/array_ref.blessed.msvc)
endif()
cinch_add_unit(array_ref
  SOURCES test/array_ref.cc common.cc
  INPUTS  test/array_ref.blessed ${array_ref_blessed_input}
  FOLDER "Tests/Util"
)

set(common_blessed_input test/common.blessed.gnug)
if(MSVC)
  set(common_blessed_input test/common.blessed.msvc)
endif()
cinch_add_unit(common
  SOURCES common.cc
          test/common.cc
  INPUTS  test/common.blessed.ppc test/common.blessed ${common_blessed_input}
  FOLDER "Tests/Util"
)

cinch_add_unit(debruijn
  SOURCES debruijn.cc
          test/debruijn.cc
  FOLDER "Tests/Util"
)

set(id_blessed_input test/id.blessed.gnug)
if(MSVC)
  set(id_blessed_input test/id.blessed.msvc)
endif()
cinch_add_unit(id
  SOURCES test/id.cc common.cc
  INPUTS  test/id.blessed ${id_blessed_input}
  FOLDER "Tests/Util"
)

if(ENABLE_OPENSSL)
  cinch_add_unit(checksum
    SOURCES
      test/checksum.cc
    LIBRARIES
      ${OPENSSL_LIBRARIES}
    FOLDER
      "Tests/Util"
  )
endif()
