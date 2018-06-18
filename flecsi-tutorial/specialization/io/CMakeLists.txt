#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

if(ENABLE_FLECSI_TUTORIAL_VTK)

  set(io_HEADERS
    vtk/rectilinearGrid.h
    vtk/unstructuredGrid.h
    vtk/vtkMPI.h
    vtk/structuredGrid.h
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(io_HEADERS
    ${io_HEADERS}
    PARENT_SCOPE
)

set(io_SOURCES
  PARENT_SCOPE
)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
