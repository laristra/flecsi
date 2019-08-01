find_program(BREATHE_APIDOC NAMES breathe-apidoc
    DOC "Breathe extension for Sphinx"
)

if(BREATHE_APIDOC)
    execute_process(COMMAND ${BREATHE_APIDOC} --version OUTPUT_VARIABLE BREATHE_VERSION_OUTPUT)
    if("${BREATHE_VERSION_OUTPUT}" MATCHES "^Breathe \\(breathe-apidoc\\) ([^\n]+)\n")
      set(BREATHE_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Breathe REQUIRED_VARS BREATHE_APIDOC
    VERSION_VAR BREATHE_VERSION
)

mark_as_advanced(BREATHE_APIDOC)
