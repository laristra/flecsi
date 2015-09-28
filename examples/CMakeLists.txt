#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add a rule to build the executable
#------------------------------------------------------------------------------#

#add_executable(burton burton.cc)

add_executable(burton2 burton2.cc)

#------------------------------------------------------------------------------#
# Add link dependencies
#------------------------------------------------------------------------------#

#target_link_libraries(burton ${CINCH_LINK_LIBRARIES} ${EXTRA_LIBS})

target_link_libraries(burton2 ${CINCH_LINK_LIBRARIES} ${EXTRA_LIBS})

#~---------------------------------------------------------------------------~-#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
