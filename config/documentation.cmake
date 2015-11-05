#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Create user guide header with version information
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/flexi_ug_header.tex.in
    ${CMAKE_BINARY_DIR}/doc/flexi_ug_header.tex)

#------------------------------------------------------------------------------#
# Pandoc options for user guide
#------------------------------------------------------------------------------#

set(ug_pandoc_options
    "--toc"
    "--include-in-header=${CMAKE_SOURCE_DIR}/cinch/tex/addtolength.tex"
    "--include-in-header=${CMAKE_BINARY_DIR}/doc/flexi_ug_header.tex"
    "--include-in-header=${CMAKE_CURRENT_SOURCE_DIR}/doc/flexi_ug_title.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/maketitle.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/firstpageempty.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/titlebreak.tex"
)

#------------------------------------------------------------------------------#
# Add user guide target
#------------------------------------------------------------------------------#

cinch_add_doc(user-guide flexi_ug.py src
    flexi-user-guide-${${PROJECT_NAME}_VERSION}.pdf
    PANDOC_OPTIONS ${ug_pandoc_options} IMAGE_GLOB "*.pdf")

#------------------------------------------------------------------------------#
# Create developer guide header with version information
#------------------------------------------------------------------------------#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/flexi_dg_header.tex.in
    ${CMAKE_BINARY_DIR}/doc/flexi_dg_header.tex)

#------------------------------------------------------------------------------#
# Pandoc options for developer guide
#------------------------------------------------------------------------------#

set(dg_pandoc_options
    "--toc"
    "--include-in-header=${CMAKE_SOURCE_DIR}/cinch/tex/addtolength.tex"
    "--include-in-header=${CMAKE_BINARY_DIR}/doc/flexi_dg_header.tex"
    "--include-in-header=${CMAKE_CURRENT_SOURCE_DIR}/doc/flexi_dg_title.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/maketitle.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/firstpageempty.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/titlebreak.tex"
)

#------------------------------------------------------------------------------#
# Add developer guide target
#------------------------------------------------------------------------------#

cinch_add_doc(developer-guide flexi_dg.py src
    flexi-developer-guide-${${PROJECT_NAME}_VERSION}.pdf
    PANDOC_OPTIONS ${dg_pandoc_options} IMAGE_GLOB "*.pdf")

#------------------------------------------------------------------------------#
# Pandoc options for charter
#------------------------------------------------------------------------------#

set(charter_pandoc_options
    "--include-in-header=${CMAKE_SOURCE_DIR}/cinch/tex/addtolength.tex"
)

#------------------------------------------------------------------------------#
# Add charter target
#------------------------------------------------------------------------------#

cinch_add_doc(charter flexi_charter.py src
  flexi-charter-${${PROJECT_NAME}_VERSION}.pdf
  PANDOC_OPTIONS ${charter_pandoc_options})

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
