#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

file(GLOB TEXFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tex/*.tex)
file(GLOB BIBFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} bib/*.bib)

add_latex_document(
  flecsi-2.tex
  INPUTS ${TEXFILES}
  #IMAGE_DIRS images
  BIBFILES ${BIBFILES}
)
