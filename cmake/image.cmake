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

function(add_image target tag dockerfile)

  set(options)
  set(one_value_args)
  set(multi_value_args ARGS DEPENDS)

  cmake_parse_arguments(image "${options}" "${one_value_args}"
    "${multi_value_args}" ${ARGN})

  foreach(arg ${image_ARGS})
    list(APPEND args "--build-arg;${arg}")
  endforeach()

  add_custom_target(${target}
    ${ENGINE} build \${EXTRA} ${args} -t ${tag}
      -f ${CMAKE_SOURCE_DIR}/${dockerfile} .
    DEPENDS ${dockerfile} ${image_DEPENDS})

endfunction()
