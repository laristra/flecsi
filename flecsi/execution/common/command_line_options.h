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

#define FLECSI_FLOG_TAG_OPTION                                                 \
  ("flog-tags",                                                                \
    value(&__flecsi_tags)->implicit_value("0"),                                \
    "Enable the specified output tags, e.g., --flog-tags=tag1,tag2."           \
    " Passing --flog-tags by itself will print the available tags.")

#define FLECSI_THREADS_PER_SHARD_OPTION                                        \
  ("runtime-threads-per-shard",                                                \
    value<size_t>()->default_value(1),                                         \
    "Specify the number of threads per shard.")
