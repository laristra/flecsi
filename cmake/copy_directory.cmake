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
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#------------------------------------------------------------------------------#

include(subdirlist)
include(subfilelist)

function(copy_directory src dst)

    # Find the files in the top-level directory
    make_subfilelist(file_list ${src})

    # Copy files
    foreach(file ${file_list})
        file(COPY ${src}/${file}
            DESTINATION ${CMAKE_BINARY_DIR}/${dst})
    endforeach(file)

    # Find all of the subdirectories of the tree
    make_subdirlist(dir_list ${src} TRUE)

    # Recurse the tree
    foreach(dir ${dir_list})

        # Create subdirectories
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${dst}/${dir})

        # Find all of the files in this subdirectory
        make_subfilelist(file_list ${src}/${dir})

        # Copy files
        foreach(file ${file_list})
            file(COPY ${src}/${dir}/${file}
                DESTINATION ${CMAKE_BINARY_DIR}/${dst}/${dir})
        endforeach(file)

    endforeach(dir)

endfunction()
