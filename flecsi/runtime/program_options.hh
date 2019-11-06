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

#include <flecsi-config.h>

#include <flecsi/execution.hh>

#include <boost/program_options.hpp>

#include <limits>
#include <string>

#if defined(FLECSI_ENABLE_FLOG)

/*
  Command-line option to display flog available tags.
 */

inline auto flecsi_flog_tags_option =
  flecsi::add_program_option("FleCSI Options",
    "flog-tags",
    boost::program_options::value(
      &flecsi::runtime::context_t::instance().flog_tags())
      ->implicit_value("0")
      ->default_value("all"),
    "Enable the specified output tags, e.g., --flog-tags=tag1,tag2."
    " Passing --flog-tags by itself will print the available tags.");

/*
  Command-line option to set verbose flog output.
 */

inline auto flecsi_flog_verbose_option =
  flecsi::add_program_option("FleCSI Options",
    "flog-verbose",
    boost::program_options::value(
      &flecsi::runtime::context_t::instance().flog_verbose())
      ->implicit_value(1)
      ->default_value(0),
    "Enable verbose output. Passing '-1' will strip any additional"
    "decorations added by flog and will only output the user's message.");

/*
  Command-line option to restrict flog output to a single process.
 */

inline auto flecsi_flog_output_process_option =
  flecsi::add_program_option("FleCSI Options",
    "flog-process",
    boost::program_options::value(
      &flecsi::runtime::context_t::instance().flog_output_process())
      ->default_value(std::numeric_limits<size_t>::max()),
    "Restrict output to the specified process id.");

#endif // FLECSI_ENABLE_FLOG
