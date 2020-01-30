
if(ENABLE_SPHINX AND ENABLE_DOXYGEN)

  find_package(Git)

  if(NOT GIT_FOUND)
    message(FATAL_ERROR "Git is required for this target")
  endif()

  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/deploy-output"
    COMMAND
      echo "Building Sphinx" && make sphinx >
        ${CMAKE_BINARY_DIR}/deploy-output 2>&1
    COMMAND
      echo "Building Doxygen" && make doxygen >>
        ${CMAKE_BINARY_DIR}/deploy-output 2>&1
    COMMAND
      echo "Updating gh-pages" &&
        [ -e gh-pages ] ||
          ${GIT_EXECUTABLE} clone --branch gh-pages
            git@gitlab.lanl.gov:laristra/flecsi.git gh-pages >>
        ${CMAKE_BINARY_DIR}/deploy-output 2>&1
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  add_custom_target(deploy-documentation
    DEPENDS "${CMAKE_BINARY_DIR}/deploy-output"
    COMMAND
      echo "Updateing Sphinx pages" &&
        cp -rT doc/sphinx gh-pages
    COMMAND
      echo "Updateing Doxygen pages" &&
        cp -rT doc/doxygen/html gh-pages/doxygen
    COMMAND
      echo "Updated gh-pages are in ${CMAKE_BINARY_DIR}/gh-pages"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

endif()
