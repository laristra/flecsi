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

macro(make_subdirlist result directory recursive)

    if(${recursive})
        file(GLOB_RECURSE children RELATIVE ${directory} ${directory}/*)
    else()         
        file(GLOB children RELATIVE ${directory} ${directory}/*)
    endif(${recursive})

    set(dir_list "")

    foreach(child ${children})
        if(NOT IS_DIRECTORY ${directory}/${child})
            get_filename_component(dir ${child} PATH) 
        else()
            set(dir ${child})
        endif(NOT IS_DIRECTORY ${directory}/${child})

        set(dir_list ${dir_list} ${dir})
    endforeach()

    if(dir_list)
      list(REMOVE_DUPLICATES dir_list)    
    endif()

    set(${result} ${dir_list})
endmacro(make_subdirlist)
