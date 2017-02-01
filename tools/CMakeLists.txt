#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Mesh generation utility
#------------------------------------------------------------------------------#

add_executable(flecsi-mg mesh-gen/main.cc)

#------------------------------------------------------------------------------#
# Collect information for FleCSIT
#------------------------------------------------------------------------------#

# Get the compiler defines that were used to build the library
# to pass to the flecsit script
get_directory_property(_defines DIRECTORY ${CMAKE_SOURCE_DIR}
  COMPILE_DEFINITIONS)
get_directory_property(_includes DIRECTORY ${CMAKE_SOURCE_DIR}
  INCLUDE_DIRECTORIES)

# Create string of compiler definitions for script
list(REMOVE_DUPLICATES _defines)
set(FLECSIT_COMPILE_DEFINES)
foreach(def ${_defines})
  set(FLECSIT_COMPILE_DEFINES
    "${FLECSIT_COMPILE_DEFINES} -D${def}")
endforeach()

string(STRIP "${FLECSIT_COMPILE_DEFINES}" FLECSIT_COMPILE_DEFINES)

# Create string of include directories for script
list(REMOVE_DUPLICATES _includes)
set(FLECSIT_INCLUDE_DIRECTORIES)
foreach(inc ${_includes})
  set(FLECSIT_INCLUDE_DIRECTORIES
    "${FLECSIT_INCLUDE_DIRECTORIES} -I${inc}")
endforeach()

string(STRIP "${FLECSIT_INCLUDE_DIRECTORIES}" FLECSIT_INCLUDE_DIRECTORIES)

# Create string of runtime link libraries for script
# Create list of link directories for LD_LIBRARY_PATH hint
set(FLECSIT_RUNTIME_LIBRARIES)
set(FLECSIT_LD_LIBRARY_PATH)
foreach(lib ${FLECSI_RUNTIME_LIBRARIES})
  # Runtime link libraries
  set(FLECSIT_RUNTIME_LIBRARIES
    "${FLECSIT_RUNTIME_LIBRARIES} ${lib}")

  # LD_LIBRARY_PATH hint
  get_filename_component(_path ${lib} DIRECTORY)
  list(APPEND FLECSIT_LD_LIBRARY_PATH ${_path})
endforeach()

string(STRIP "${FLECSI_RUNTIME_LIBRARIES}" FLECSI_RUNTIME_LIBRARIES)

# Append local build and remove duplicates
list(APPEND FLECSIT_LD_LIBRARY_PATH ${CMAKE_BINARY_DIR}/lib)
list(REMOVE_DUPLICATES FLECSIT_LD_LIBRARY_PATH)

string(STRIP "${FLECSIT_LD_LIBRARY_PATH}" FLECSIT_LD_LIBRARY_PATH)

#------------------------------------------------------------------------------#
# FleCSIT
#------------------------------------------------------------------------------#

set(FLECSI_PYTHON_PATH_MODULE)
set(FLECSI_PYTHON_PATH_BASH)
set(FLECSI_PYTHON_PATH_CSH)

if(ENABLE_FLECSIT)

	find_package(PythonInterp 2.7 REQUIRED)

	execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig as cg; print cg.get_python_lib(0,0,prefix='${CMAKE_INSTALL_PREFIX}')" OUTPUT_VARIABLE PYTHON_INSTDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

	install(DIRECTORY ${CMAKE_SOURCE_DIR}/tools/flecsit/flecsit
		DESTINATION ${PYTHON_INSTDIR}
		FILES_MATCHING PATTERN "*.py")

	configure_file(${CMAKE_SOURCE_DIR}/tools/flecsit/bin/flecsit.in
		${CMAKE_BINARY_DIR}/flecsit/bin/flecsit @ONLY)

	install(PROGRAMS ${CMAKE_BINARY_DIR}/flecsit/bin/flecsit
		DESTINATION bin
		PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE
			WORLD_READ WORLD_EXECUTE
	)

  set(FLECSI_PYTHON_PATH_MODULE
    "prepend-path PYTHONPATH ${PYTHON_INSTDIR}"
  )
  set(FLECSI_PYTHON_PATH_BASH
    "export PYTHONPATH=\${PYTHONPATH}:${PYTHON_INSTDIR}"
  )
  set(FLECSI_PYTHON_PATH_CSH
    "setenv PYTHONPATH $PYTHONPATH:${PYTHON_INSTDIR}"
  )

endif()

#------------------------------------------------------------------------------#
# Handle script and source files for FleCSIT tool
#------------------------------------------------------------------------------#

# Copy the auxiliary files for local development
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/share/runtime_main.cc
  COMMAND ${CMAKE_COMMAND} -E copy
    ${PROJECT_SOURCE_DIR}/flecsi/execution/runtime_main.cc
    ${CMAKE_BINARY_DIR}/share/runtime_main.cc
    DEPENDS ${PROJECT_SOURCE_DIR}/flecsi/execution/runtime_main.cc
    COMMENT "Copying runtime main file")

add_custom_target(runtime_main ALL
  DEPENDS ${CMAKE_BINARY_DIR}/share/runtime_main.cc)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/share/runtime_driver.cc
  COMMAND ${CMAKE_COMMAND} -E copy
    ${_runtime_path}/runtime_driver.cc
    ${CMAKE_BINARY_DIR}/share/runtime_driver.cc
    DEPENDS ${_runtime_path}/runtime_driver.cc
    COMMENT "Copying runtime driver file")

add_custom_target(runtime_driver ALL
  DEPENDS ${CMAKE_BINARY_DIR}/share/runtime_driver.cc)

# Install the auxiliary files
install(FILES ${PROJECT_SOURCE_DIR}/flecsi/execution/runtime_main.cc
  DESTINATION share/flecsi/runtime)
install(FILES ${_runtime_path}/runtime_driver.cc
  DESTINATION share/flecsi/runtime)

#------------------------------------------------------------------------------#
# FleCSI environment module
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi
  DESTINATION bin
  )

#------------------------------------------------------------------------------#
# Helper shell environment setup
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.sh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.csh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh @ONLY)


# Install shell helpers
install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh
  DESTINATION bin
  RENAME flecsi.sh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh
  DESTINATION bin
  RENAME flecsi.csh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

#~---------------------------------------------------------------------------~-#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
