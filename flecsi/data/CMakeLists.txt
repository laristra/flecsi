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

set(data_HEADERS
  common/data_reference.h
  common/field_info.h
  common/privilege.h
  common/storage_class.h
  common/topology.h
  common/topology_accessor.h
  common/topology_registration.h
  data.h
  field_interface.h
  topology_instance.h
  topology_interface.h
)

set(data_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(data_HEADERS
    legion/data_policy.h
    legion/runtime_data_types.h
    legion/storage_classes.h
    ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(data_HEADERS
    mpi/data_policy.h
    mpi/storage_classes.h
    ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(data_HEADERS ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(data_HEADERS
  ${data_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(data_SOURCES
  ${data_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

ftest_add_unit(global
  SOURCES
    test/global.cc
)

ftest_add_unit(index
  SOURCES
    test/index.cc
)

ftest_add_unit(privilege
  SOURCES
    test/privilege.cc
)
