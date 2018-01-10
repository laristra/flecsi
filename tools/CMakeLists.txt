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

#------------------------------------------------------------------------------#
# These are used to set environment variables for the tutorial.
#------------------------------------------------------------------------------#

set(LIB_SUFFIX "a")

if(BUILD_SHARED_LIBS)
  set(LIB_SUFFIX "so")
endif()

set(FLECSIT_TUTORIAL_INCLUDES ${CMAKE_INSTALL_PREFIX}/include)
set(FLECSIT_TUTORIAL_LIBRARIES
  ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/libFleCSI-Tut.${LIB_SUFFIX})
set(FLECSIT_FLECSI_LIBRARY
  ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/libFleCSI.${LIB_SUFFIX})

#------------------------------------------------------------------------------#
# Create string of compiler definitions for script
#------------------------------------------------------------------------------#

list(REMOVE_DUPLICATES _defines)
set(FLECSIT_COMPILE_DEFINES)
foreach(def ${_defines})
  set(FLECSIT_COMPILE_DEFINES
    "${FLECSIT_COMPILE_DEFINES} ${def}")
endforeach()

string(STRIP "${FLECSIT_COMPILE_DEFINES}" FLECSIT_COMPILE_DEFINES)

#------------------------------------------------------------------------------#
# Create string of include directories for script
#------------------------------------------------------------------------------#

list(REMOVE_DUPLICATES _includes)
set(FLECSIT_INCLUDES)
foreach(inc ${_includes})
  set(FLECSIT_INCLUDES
    "${FLECSIT_INCLUDES} ${inc}")
endforeach()

string(STRIP "${FLECSIT_INCLUDES}" FLECSIT_INCLUDES)

#------------------------------------------------------------------------------#
# Create string of runtime link libraries for script
# Create list of link directories for LD_LIBRARY_PATH hint
#------------------------------------------------------------------------------#

set(FLECSIT_LIBRARIES)
set(FLECSIT_LDFLAGS)
set(FLECSI_LD_DEPENDS_LIST)
foreach(lib ${FLECSI_LIBRARY_DEPENDENCIES})
  # Runtime link libraries
  set(FLECSIT_LIBRARIES
    "${FLECSIT_LIBRARIES} ${lib}")

  # LD_LIBRARY_PATH hint
  get_filename_component(_path ${lib} DIRECTORY)
  set(FLECSIT_LDFLAGS "${FLECSIT_LDFLAGS} -L${_path}")
  list(APPEND FLECSI_LD_DEPENDS_LIST ${_path})
endforeach()

string(STRIP "${FLECSIT_LIBRARIES}" FLECSIT_LIBRARIES)

list(REMOVE_DUPLICATES FLECSI_LD_DEPENDS_LIST)

#------------------------------------------------------------------------------#
# Process the list of path dependencies for environment module LD_LIBARRY_PATH
#------------------------------------------------------------------------------#

set(FLECSI_LD_DEPENDS)
foreach(lib ${FLECSI_LD_DEPENDS_LIST})
  if(NOT FLECSI_LD_DEPENDS)
    set(FLECSI_LD_DEPENDS "${lib}")
  else()
    set(FLECSI_LD_DEPENDS "${FLECSI_LD_DEPENDS}:${lib}")
  endif()
endforeach()

# Append local build and remove duplicates
set(FLECSIT_LDFLAGS "${FLECSIT_LDFLAGS} -L${CMAKE_BINARY_DIR}/lib")

string(STRIP "${FLECSIT_LDFLAGS}" FLECSIT_LDFLAGS)

#------------------------------------------------------------------------------#
# FleCSIT
#------------------------------------------------------------------------------#

set(FLECSI_PYTHON_PATH_MODULE)
set(FLECSI_PYTHON_PATH_BASH)
set(FLECSI_PYTHON_PATH_CSH)

if(ENABLE_FLECSIT)

    #--------------------------------------------------------------------------#
    # FleCSIT dependencies
    #--------------------------------------------------------------------------#

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
    ${_runtime_path}/runtime_main.cc
    ${CMAKE_BINARY_DIR}/share/runtime_main.cc
    DEPENDS ${_runtime_path}/runtime_main.cc
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
install(FILES ${_runtime_path}/runtime_main.cc
  DESTINATION ${FLECSI_SHARE_DIR}/runtime)
install(FILES ${_runtime_path}/runtime_driver.cc
  DESTINATION ${FLECSI_SHARE_DIR}/runtime)

#------------------------------------------------------------------------------#
# FleCSI environment module
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi
  DESTINATION bin
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi-tutorial.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial
  DESTINATION bin
  )

#------------------------------------------------------------------------------#
# Helper shell environment setup
#------------------------------------------------------------------------------#

# Bash
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.sh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.sh
  DESTINATION bin
  RENAME flecsi.sh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi-tutorial.sh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial-install.sh @ONLY)

install(
  FILES
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial-install.sh
  DESTINATION bin
  RENAME flecsi-tutorial.sh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

# Csh
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.csh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh @ONLY)

install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install.csh
  DESTINATION bin
  RENAME flecsi.csh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi-tutorial.csh.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial-install.csh @ONLY)

install(
  FILES
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-tutorial-install.csh
  DESTINATION bin
  RENAME flecsi-tutorial.csh
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)

#~---------------------------------------------------------------------------~-#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#~---------------------------------------------------------------------------~-#
