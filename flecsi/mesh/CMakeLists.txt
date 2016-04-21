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

set(mesh_HEADERS
  mesh_topology.h
  mesh_types.h
  mesh_utils.h
  PARENT_SCOPE)

set(mesh_SOURCES
  PARENT_SCOPE)

cinch_add_unit(traversal
  SOURCES test/traversal.cc
  INPUTS test/traversal.blessed)

cinch_add_unit(dual
  SOURCES test/dual.cc
  INPUTS test/dual.blessed)

cinch_add_unit(bindings
  SOURCES test/bindings.cc
  INPUTS test/bindings.blessed)

cinch_add_unit(dolfin_triangle
        SOURCES test/dolfin_triangle/test/dolfin_triangle.cc
        INPUTS  test/dolfin_triangle/test/dolfin_triangle.blessed
        )
cinch_add_unit(dolfin_triangle_mesh_partition
        SOURCES test/dolfin_triangle/test/dolfin_triangle_mesh_partition.cc
        )
cinch_add_unit(dolfin_triangle_connectivity
        SOURCES test/dolfin_triangle/test/dolfin_triangle_connectivity.cc
        )

if (ENABLE_PARTITION AND METIS_FOUND)
    cinch_add_unit(dolfin_triangle_metis
            SOURCES test/dolfin_triangle/test/metis_partition_mesh_dual.cc
            LIBRARIES ${PARTITION_LIBRARIES})
    cinch_add_unit(metis_partition_mesh_nodal
            SOURCES test/dolfin_triangle/test/metis_partition_mesh_nodal.cc
            LIBRARIES ${PARTITION_LIBRARIES})
endif()

cinch_add_unit(sagittarius
        SOURCES test/sagittarius/test/sagittarius.cc
        INPUTS  test/sagittarius/test/sagittarius.blessed
        )
cinch_add_unit(sagittarius_mesh_partition
        SOURCES test/sagittarius/test/sagittarius_mesh_partition.cc
        )

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
