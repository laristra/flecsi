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

set(topology_HEADERS
  closure_utils.h
  color_topology.h
  common/array_buffer.h
  common/entity_storage.h
  connectivity.h
  entity_storage.h
  index_space.h
  mesh_definition.h
  parallel_mesh_definition.h
  mesh.h
  mesh_storage.h
  mesh_topology.h
  global_topology.h
  mesh_types.h
  mesh_utils.h
  partition.h
  set_storage.h
  set_topology.h
  set_types.h
  set_utils.h
  structured_mesh_topology.h
  tree_topology.h
  types.h
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)
  set(topology_HEADERS
    ${topology_HEADERS}
    legion/set_storage_policy.h
    legion/storage_policy.h
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
  set(topology_HEADERS
    ${topology_HEADERS}
    mpi/set_storage_policy.h
    mpi/storage_policy.h
    )

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(topology_HEADERS
    ${topology_HEADERS}
    hpx/storage_policy.h
    )

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER ../execution/hpx/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(topology_HEADERS
    ${topology_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(topology_SOURCES
    ${topology_SOURCES}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Mesh unit tests.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  cinch_add_unit(traversal
   SOURCES
     test/traversal.cc
   INPUTS
     test/traversal.blessed
   LIBRARIES
     FleCSI
  )

  cinch_add_unit(bindings
   SOURCES
     test/bindings.cc
   INPUTS
     test/bindings.blessed
   LIBRARIES
     FleCSI
  )

endif()

cinch_add_unit(dual
  SOURCES
    test/dual.cc
  INPUTS
    test/dual.blessed
)

#------------------------------------------------------------------------------#
# N-Tree unit tests.
#------------------------------------------------------------------------------#

#cinch_add_unit(tree
#  SOURCES
#    test/tree.cc
#    test/pseudo_random.h
#  INPUTS
#    test/tree.blessed
#  LIBRARIES
#    FleCSI
#)

#cinch_add_unit(tree1d
#  SOURCES
#    test/tree1d.cc
#  LIBRARIES
#    FleCSI
#)

#cinch_add_unit(tree3d
#  SOURCES
#    test/tree3d.cc
#  LIBRARIES
#    FleCSI
#)

#cinch_add_unit(gravity
#  SOURCES
#    test/gravity.cc test/pseudo_random.h
#  LIBRARIES
#    FleCSI
#)

# FIXME: Broken by refactor
#cinch_add_unit(gravity-state
#  SOURCES
#    test/gravity-state.cc
#    test/pseudo_random.h
#  LIBRARIES
#    FleCSI
#    ${CINCH_RUNTIME_LIBRARIES}
#)

cinch_add_unit(structured
  SOURCES
    test/structured.cc
)

#------------------------------------------------------------------------------#
# Set unit tests.
#------------------------------------------------------------------------------#

# cinch_add_unit(set_topology
#   SOURCES
#     test/set_topology.cc
# )

#------------------------------------------------------------------------------#
# Misc. unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(closure
  SOURCES
    test/closure.cc
)

cinch_add_unit(devel-closure
  SOURCES
    test/devel-closure.cc
  POLICY
    SERIAL_DEVEL
)

# FIXME: Check this with refactor
#cinch_add_unit(index-space
#  SOURCES
#    test/index-space.cc test/pseudo_random.h
#)

