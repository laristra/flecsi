/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file defines common command-line options for use with
  boost::program_options.
 */

#include <limits>
#include <string>

inline std::string __flog_tags;
inline int __flog_verbose;
inline size_t __flog_process;

/*
  Command-line option to display flog available tags.
 */

#define FLECSI_FLOG_TAG_OPTION                                                 \
  ("flog-tags",                                                                \
    value(&__flog_tags)->implicit_value("0")->default_value("all"),            \
    "Enable the specified output tags, e.g., --flog-tags=tag1,tag2."           \
    " Passing --flog-tags by itself will print the available tags.")

/*
  Command-line option to set verbose flog output.
 */

#define FLECSI_FLOG_VERBOSE_OPTION                                             \
  ("flog-verbose",                                                             \
    value(&__flog_verbose)->implicit_value(1)->default_value(0),        \
    "Enable verbose output. Passing '-1' will strip any additional" \
    "decorations added by flog and will only output the user's message.")

/*
  Command-line option to restrict flog output to a single process.
 */

#define FLECSI_FLOG_PROCESS_OPTION                                             \
  ("flog-process",                                                             \
    value(&__flog_process)->default_value(std::numeric_limits<size_t>::max()), \
    "Restrict output to the specified process id.")

/*
  Command-line option to specify threads per process.
 */

#define FLECSI_TPP_OPTION_STRING "tpp"

#define FLECSI_THREADS_PER_PROCESS_OPTION                                      \
  (FLECSI_TPP_OPTION_STRING,                                                   \
    value<size_t>()->default_value(1),                                         \
    "Specify the number of threads per process.")
