/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
/*! @file */

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <flecsi/execution/context.h>

// Boost command-line options
#if defined(FLECSI_ENABLE_BOOST)
#include <boost/program_options.hpp>
using namespace boost::program_options;
#endif

/*!
 FleCSI runtime main function.
 */
int
main(int argc, char ** argv) {

  // Initialize the MPI runtime
  MPI_Init(&argc, &argv);

  // get the rank
  int rank{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //--------------------------------------------------------------------------//
  // INIT CLOG
  //--------------------------------------------------------------------------//

  // Initialize tags to output all tag groups from CLOG
  std::string tags{"all"};

#if defined(FLECSI_ENABLE_BOOST)
  options_description desc("FleCSI runtime options");

  // Add command-line options
  desc.add_options()("help,h", "Print this message and exit.")("tags,t",
    value(&tags)->implicit_value("0"),
    "Enable the specified output tags, e.g., --tags=tag1,tag2."
    " Passing --tags by itself will print the available tags.");
  variables_map vm;
  parsed_options parsed =
    command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  store(parsed, vm);

  notify(vm);

  if(vm.count("help")) {
    if(rank == 0) {
      std::cout << desc << std::endl;
    } // if

    return 1;
  } // if

#endif // FLECSI_ENABLE_BOOST

  int result{0};

  if(tags == "0") {
    // Output the available tags
    if(rank == 0) {
      std::cout << "Available tags (CLOG):" << std::endl;

      for(auto t : clog_tag_map()) {
        std::cout << "  " << t.first << std::endl;
      } // for
    } // if
  }
  else {
    // Initialize the cinchlog runtime
    clog_init(tags);

    // Execute the flecsi runtime.
    result = flecsi::execution::context_t::instance().initialize(argc, argv);
    flecsi::execution::context_t::instance().finalize();
  } // if

  // Shutdown the MPI runtime
  MPI_Finalize();

  return result;
} // main
