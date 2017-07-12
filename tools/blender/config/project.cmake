#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

project(blender)

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

cinch_add_application_directory(bin)

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(blender blender)

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

#------------------------------------------------------------------------------#
# LLVM
#------------------------------------------------------------------------------#

find_package(LLVM 3.7 REQUIRED)

#------------------------------------------------------------------------------#
# Clang
#------------------------------------------------------------------------------#

find_package(Clang
  REQUIRED
  COMPONENTS
    FrontendTool
    Frontend
    Driver
    Serialization
    CodeGen
    Parse
    Sema
    RewriteFrontend
    Rewrite
    StaticAnalyzerFrontend
    StaticAnalyzerCheckers
    StaticAnalyzerCore
    ToolingCore
    Tooling
    ARCMigrate
    Analysis
    Edit
    AST
    ASTMatchers
    Lex
    Basic
    BitReader
)

target_include_directories(blender PRIVATE ${CLANG_INCLUDE_DIR})
target_link_libraries(blender ${CLANG_LIBRARIES} ${LLVM_LIBRARIES})
target_compile_options(blender PRIVATE -fno-rtti)

message(STATUS "Clang version: ${CLANG_VERSION}")
message(STATUS "Clang include directory: ${CLANG_INCLUDE_DIR}")

# What does this do?
#string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
#  CLANG_VERSION_STRING ${clang_full_version_string})

#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib/clang/${CLANG_VERSION_STRING})

#EXECUTE_PROCESS(COMMAND ${LLVM_CONFIG} --libdir
#  OUTPUT_VARIABLE llvm_lib_dir OUTPUT_STRIP_TRAILING_WHITESPACE )

#string (REGEX REPLACE "\n$" "" LLVM_LIB_DIR ${llvm_lib_dir})

#add_custom_target(clang-include-dir-link ALL
#    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LLVM_LIB_DIR}/clang/${CLANG_VERSION_STRING}/include lib/clang/${CLANG_VERSION_STRING}/include)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
