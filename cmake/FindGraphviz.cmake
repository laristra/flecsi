# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find Graphviz
# Find the native Graphviz headers and libraries.
#
# Graphviz_INCLUDE_DIRS	- where to find m_apm.h, etc.
# Graphviz_LIBRARIES	- List of libraries when using Graphviz.
# Graphviz_FOUND	- True if Graphviz found.

find_package(PkgConfig)

pkg_check_modules(PC_Graphviz graphviz)

# Look for the header file.
FIND_PATH(Graphviz_INCLUDE_DIR NAMES graphviz/cgraph.h)

# Look for the library.
FIND_LIBRARY(Graphviz_cdt_LIBRARY NAMES cdt )
FIND_LIBRARY(Graphviz_cgraph_LIBRARY NAMES cgraph )
FIND_LIBRARY(Graphviz_gvc_LIBRARY NAMES gvc )
FIND_LIBRARY(Graphviz_gvpr_LIBRARY NAMES gvpr )
FIND_LIBRARY(Graphviz_pathplan_LIBRARY NAMES pathplan )
FIND_LIBRARY(Graphviz_xdot_LIBRARY NAMES xdot )

SET(Graphviz_LIBRARY
	${Graphviz_cdt_LIBRARY}
	${Graphviz_cgraph_LIBRARY}
	${Graphviz_gvc_LIBRARY}
	${Graphviz_gvpr_LIBRARY}
	${Graphviz_pathplan_LIBRARY}
	${Graphviz_xdot_LIBRARY}
)

# Handle the QUIETLY and REQUIRED arguments and set Graphviz_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Graphviz DEFAULT_MSG 
	Graphviz_LIBRARY Graphviz_INCLUDE_DIR)

# Copy the results to the output variables.
IF(Graphviz_FOUND)
	SET(Graphviz_LIBRARIES ${Graphviz_LIBRARY})
	SET(Graphviz_INCLUDE_DIRS ${Graphviz_INCLUDE_DIR})
ELSE(Graphviz_FOUND)
	SET(Graphviz_LIBRARIES)
	SET(Graphviz_INCLUDE_DIRS)
ENDIF(Graphviz_FOUND)

MARK_AS_ADVANCED(Graphviz_INCLUDE_DIRS Graphviz_INCLUDE_DIR Graphviz_LIBRARIES Graphviz_cdt_LIBRARY Graphviz_cgraph_LIBRARY Graphviz_gvc_LIBRARY Graphviz_gvpr_LIBRARY Graphviz_pathplan_LIBRARY Graphviz_xdot_LIBRARY)
