#
# FindClangFormat
# ---------------
#
# The module defines the following variables
#
# ClangFormat_EXECUTABLE   - Path to clang-format executable
# ClangFormat_FOUND        - True if the clang-format executable was found.
# ClangFormat_VERSION      - The version of clang-format found
#

set(ClangFormat_EXECUTABLE "" CACHE STRING
  "Explicitly set the clang-format executable to use")
mark_as_advanced(ClangFormat_EXECUTABLE)

if(ClangFormat_EXECUTABLE STREQUAL "")

  find_program(find_clangformat
    NAMES
      clang-format
      clang-format-10
      clang-format-9
      clang-format-8
      clang-format-7
      clang-format-6.0
      clang-format-5.0
      clang-format-4.0
      clang-format-3.9
      clang-format-3.8
      clang-format-3.7
      clang-format-3.6
      clang-format-3.5
      clang-format-3.4
      clang-format-3.3
    DOC
      "clang-format executable"
  )

  set(ClangFormat_EXECUTABLE ${find_clangformat})
  mark_as_advanced(find_clangformat)
endif()

# Extract version from command "clang-format -version"
if(ClangFormat_EXECUTABLE)
  execute_process(COMMAND ${ClangFormat_EXECUTABLE} -version
    OUTPUT_VARIABLE clang_format_version
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(clang_format_version MATCHES "^clang-format version .*")
    # clang_format_version sample: "clang-format version 3.9.1-4ubuntu3~16.04.1
    # (tags/RELEASE_391/rc2)"
    string(REGEX
      REPLACE "clang-format version ([.0-9]+).*"
      "\\1"
      ClangFormat_VERSION
      "${clang_format_version}")
  else()
    set(ClangFormat_VERSION 0.0)
  endif()
else()
  set(ClangFormat_VERSION 0.0)
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set ClangFormat_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ClangFormat
  REQUIRED_VARS
    ClangFormat_EXECUTABLE
    ClangFormat_VERSION
)
