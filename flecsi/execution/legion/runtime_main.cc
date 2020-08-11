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

#if defined(ENABLE_CALIPER)
#include <caliper/cali-mpi.h>
#include <caliper/cali.h>
#endif

//----------------------------------------------------------------------------//
//! FleCSI runtime main function.
//----------------------------------------------------------------------------//

int
main(int argc, char ** argv) {

#if defined(FLECSI_ENABLE_MPI)
#if defined(GASNET_CONDUIT_MPI) || defined(REALM_USE_MPI)
  // We require MPI support THREAD_MULTIPLE for the Legion runtime
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  // If you fail this assertion, then your version of MPI
  // does not support calls from multiple threads and you
  // cannot use the GASNet MPI conduit
  if(provided < MPI_THREAD_MULTIPLE)
    printf("ERROR: Your implementation of MPI does not support "
           "MPI_THREAD_MULTIPLE which is required for use of the "
           "GASNet MPI conduit with the Legion-MPI Interop!\n");
  assert(provided == MPI_THREAD_MULTIPLE);

#else
  clog_error("LEGION+GAsnet should be configured with MPI support");
#endif

  // get the rank
  int rank{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#endif // FLECSI_ENABLE_MPI

  //--------------------------------------------------------------------------//
  // INIT CLOG
  //--------------------------------------------------------------------------//

  // Initialize tags to output all tag groups from CLOG
  std::string tags("all");

#if defined(FLECSI_ENABLE_BOOST)
  options_description desc("Cinch test options");

  // Add command-line options
  desc.add_options()("help,h", "Print this message and exit.")("tags,t",
    value(&tags)->implicit_value("0"),
    "Enable the specified output tags, e.g., --tags=tag1,tag2."
    " Passing --tags by itself will print the available tags.");
  std::string leg_args;
  desc.add_options()("backend-args", value(&leg_args)->default_value(""),
    "Pass arguments to the runtime backend. The single argument is a quoted "
    "string of backend-specific options.");
  variables_map vm;
  parsed_options parsed =
    command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  store(parsed, vm);

  notify(vm);

  if(vm.count("help")) {
    if(rank == 0) {
      std::cout << desc << std::endl;
    } // if

    MPI_Finalize();
    return 1;
  } // if

  { // store legion arguments in context
    auto iss = std::istringstream{leg_args};
    std::vector<std::string> lsargs(std::istream_iterator<std::string>{iss},
      std::istream_iterator<std::string>());
    for(auto & arg : lsargs) {
      flecsi::execution::context_t::instance().add_backend_arg(arg);
    }
  }

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

  } // if

  return result;
} // main
