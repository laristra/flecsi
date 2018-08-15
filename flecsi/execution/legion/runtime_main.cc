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
#if defined(FLECSI_ENABLE_BOOST_PROGRAM_OPTIONS)
  #include <boost/program_options.hpp>
  using namespace boost::program_options;
#endif

//----------------------------------------------------------------------------//
//! FleCSI runtime main function.
//----------------------------------------------------------------------------//

int main(int argc, char ** argv) {

  int rank(0);

#if defined(FLECSI_ENABLE_MPI)
  // Get the MPI version
  int version, subversion;
  MPI_Get_version(&version, &subversion);

#if defined(GASNET_CONDUIT_MPI)
  if(version==3 && subversion>0) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // If you fail this assertion, then your version of MPI
    // does not support calls from multiple threads and you
    // cannot use the GASNet MPI conduit
    if (provided < MPI_THREAD_MULTIPLE)
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

  // get the rank
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#endif // FLECSI_ENABLE_MPI
  
  //--------------------------------------------------------------------------//
  // INIT CLOG
  //--------------------------------------------------------------------------//
  
  // Initialize tags to output all tag groups from CLOG
  std::string tags("all");
  bool help = false;

  //--------------------------------------------------------------------------//
  // Use BOOST Program Options

#if defined(FLECSI_ENABLE_BOOST_PROGRAM_OPTIONS)
  options_description desc("Cinch test options");  

  // Add command-line options
  desc.add_options()
    ("help,h", "Print this message and exit.")
    ("tags,t", value(&tags)->implicit_value("0"),
     "Enable the specified output tags, e.g., --tags=tag1,tag2."
     " Passing --tags by itself will print the available tags.")
    ;
  variables_map vm;
  parsed_options parsed =
    command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  store(parsed, vm);

  notify(vm);

  // was help requested
  help = vm.count("help");
  if(help) {
    if(rank == 0) {
      std::cout << desc << std::endl;
    } // if
    // don't exit, because the user application
    // may want to print a usage message too
  } // if

  
#endif // FLECSI_ENABLE_BOOST_PROGRAM_OPTIONS

  // End BOOST Program Options
  //--------------------------------------------------------------------------//

  if(tags == "0" && !help) {
    // Output the available tags
    if(rank == 0) {
      std::cout << "Available tags (CLOG):" << std::endl;

      for(auto t: clog_tag_map()) {
        std::cout << "  " << t.first << std::endl;
      } // for
    } // if
    // die nicely
#if defined(FLECSI_ENABLE_MPI)
    MPI_Finalize();
#endif
    return 0;
  }
    
  // Initialize the cinchlog runtime
  clog_init(tags);
   
   //-------------------------------------------------------------------------//
   // DONE CLOG INIT
   //-------------------------------------------------------------------------//
   

  // Execute the flecsi runtime.
  auto retval = flecsi::execution::context_t::instance().initialize(argc, argv);

#if defined(FLECSI_ENABLE_MPI)
  // FIXME: This is some kind of GASNet bug (or maybe Legion).
  // Shutdown the MPI runtime
#ifndef GASNET_CONDUIT_MPI
  MPI_Finalize();
#endif
#endif // FLECSI_ENABLE_MPI

  return retval;
} // main
