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

  add_custom_target(${target} ALL
    ${ENGINE} build \${EXTRA} ${args} -t ${tag}
      -f ${CMAKE_SOURCE_DIR}/${dockerfile} .
    DEPENDS ${dockerfile} ${image_DEPENDS})

  add_custom_target(push-${target} ALL
    ${ENGINE} push \${EXTRA} ${tag}
    DEPENDS ${target})

  add_custom_target(clean-${target}
    ${ENGINE} rmi \${EXTRA} ${tag})

endfunction()
