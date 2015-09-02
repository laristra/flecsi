#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Create user guide header with version information
#------------------------------------------------------------------------------#

configure_file(${CMAKE_SOURCE_DIR}/doc/ugheader.tex.in
    ${CMAKE_BINARY_DIR}/doc/ugheader.tex)

#------------------------------------------------------------------------------#
# Pandoc options for user guide
#------------------------------------------------------------------------------#

set(ug_pandoc_options
    "--toc"
    "--include-in-header=${CMAKE_SOURCE_DIR}/cinch/tex/addtolength.tex"
    "--include-in-header=${CMAKE_BINARY_DIR}/doc/ugheader.tex"
    "--include-in-header=${CMAKE_SOURCE_DIR}/doc/ugtitle.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/maketitle.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/firstpageempty.tex"
    "--include-before-body=${CMAKE_SOURCE_DIR}/cinch/tex/titlebreak.tex"
)

#------------------------------------------------------------------------------#
# Add user guide target
#------------------------------------------------------------------------------#

cinch_add_doc(user-guide jali_ug.py src
    jali-user-guide-${${PROJECT_NAME}_VERSION}.pdf
    PANDOC_OPTIONS ${ug_pandoc_options} IMAGE_GLOB "*.pdf")

#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
