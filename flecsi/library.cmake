#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add public library headers
#------------------------------------------------------------------------------#

message(STATUS "In library.cmake")

set(flecsi_PUBLIC_HEADERS
    flecsi_runtime_context_policy.h
    flecsi_runtime_data_handle_policy.h
    flecsi_runtime_data_policy.h
    flecsi_runtime_execution_policy.h
)

#~---------------------------------------------------------------------------~-#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
