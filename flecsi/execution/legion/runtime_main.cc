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
#include <caliper/cali.h>
#include <caliper/cali-mpi.h>
#endif 

//----------------------------------------------------------------------------//
//! FleCSI runtime main function.
//----------------------------------------------------------------------------//

int
main(int argc, char ** argv) {

#if defined(FLECSI_ENABLE_MPI)
  // Get the MPI version
  int version, subversion;
  MPI_Get_version(&version, &subversion);

#if defined(GASNET_CONDUIT_MPI)
  if(version == 3 && subversion > 0) {
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
  }
  else {
    // Initialize the MPI runtime
    MPI_Init(&argc, &argv);
  } // if
#else
  MPI_Init(&argc, &argv);
#endif

#if defined(ENABLE_CALIPER)
  cali_mpi_init();
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

#if defined(FLECSI_ENABLE_MPI)
  // Shutdown the MPI runtime
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif
#endif // FLECSI_ENABLE_MPI

  return result;
} // main
