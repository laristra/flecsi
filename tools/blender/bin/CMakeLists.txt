#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Add a rule to build the executable
#------------------------------------------------------------------------------#

add_executable(blend blend.cc)

#------------------------------------------------------------------------------#
# Add include dependencies
#------------------------------------------------------------------------------#

target_include_directories(blend
  PRIVATE ${LLVM_INCLUDE_DIR}
  PRIVATE ${CLANG_INCLUDE_DIR}
)

#------------------------------------------------------------------------------#
# Add link dependencies
#------------------------------------------------------------------------------#

target_link_libraries(blend blender ${CLANG_LIBRARIES} ${LLVM_LIBRARIES})

target_compile_options(blend PRIVATE -fno-rtti)

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
