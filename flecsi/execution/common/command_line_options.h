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

#include <string>

inline std::string __flecsi_tags = "all";

/*
  Command-line option to display flog available tags.
 */

#define FLECSI_FLOG_TAG_OPTION                                                 \
  ("flog-tags",                                                                \
    value(&__flecsi_tags)->implicit_value("0"),                                \
    "Enable the specified output tags, e.g., --flog-tags=tag1,tag2."           \
    " Passing --flog-tags by itself will print the available tags.")

/*
  Command-line option to specify threads per process.
 */

#define FLECSI_TPP_OPTION_STRING "tpp"

#define FLECSI_THREADS_PER_SHARD_OPTION                                        \
  (FLECSI_TPP_OPTION_STRING,                                                   \
    value<size_t>()->default_value(1),                                         \
    "Specify the number of threads per process.")
