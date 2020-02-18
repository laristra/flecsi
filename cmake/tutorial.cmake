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
# Copyright (c) 2016 Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

include(subdirlist)
include(subfilelist)

macro(update_sphinx_tutorial_sources input output)

  make_subdirlist(directories ${input} FALSE)

  foreach(dir ${directories})
    string(REGEX MATCH "[0-9]-\.*" match ${dir})

    if(match)
      make_subfilelist(sources ${input}/${dir})

      foreach(source ${sources})
        message(STATUS "Updating tutorial example source: ${source}")

        string(REGEX REPLACE "\.cc" "\.rst" rst_source ${source})

        set(annotated)
        file(STRINGS ${input}/${dir}/${source} lines)
        list(APPEND annotated ".. code-block:: cpp\n\n")

        foreach(line ${lines})
          list(APPEND annotated "  ${line}\n")
        endforeach()

        file(WRITE ${output}/${rst_source} ${annotated})
      endforeach()
    endif()
  endforeach()

endmacro()
