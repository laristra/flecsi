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

#include <cinchlog.h>
#include <flecsi/control/runtime.h>

#include <iostream>
#include <string>

#if defined(FLECSI_ENABLE_BOOST)
  #include <boost/program_options.hpp>
  using namespace boost::program_options;
#endif

using namespace flecsi::control;

/*!
  The main function makes use of Boost program options (optionally). These
  allow the user to control output from clog. It is also possible to add
  additional command-line options.
 */

int main(int argc, char ** argv) {

  runtime_t & runtime_ = runtime_t::instance();

  // Invoke registered runtime initializations
  runtime_.initialize_runtimes(argc, argv);

  // Initialize clog tags to output all tag groups
  std::string tags("all");

#if defined(FLECSI_ENABLE_BOOST)
  std::string program(argv[0]);
  options_description desc(program.substr(program.find('/')+1).c_str());

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

  // Gather the unregistered options, if there are any, print a help message
  // and die nicely.
  std::vector<std::string> unrecog_options =
    collect_unrecognized(parsed.options, include_positional);

  if(unrecog_options.size()) {
    if(runtime_.participate_in_output(argc, argv)) {
      std::cout << std::endl << "Unrecognized options: ";
      for ( int i=0; i<unrecog_options.size(); ++i ) {
        std::cout << unrecog_options[i] << " ";
      }
      std::cout << std::endl << std::endl << desc << std::endl;
    } // if

    return runtime_.finalize_runtimes(argc, argv,
      runtime_exit_mode_t::unrecognized_option);
  } // if

  if(vm.count("help")) {
    if(runtime_.participate_in_output(argc, argv)) {
      std::cout << desc << std::endl;
    } // if

    return runtime_.finalize_runtimes(argc, argv,
      runtime_exit_mode_t::help);
  } // if
#endif

  int result{0};

  if(tags == "0") {
    if(runtime_.participate_in_output(argc, argv)) {
      std::cout << "Available tags (CLOG): " << std::endl;

      for(auto t: clog_tag_map()) {
        std::cout << "  " << t.first << std::endl;
      } // for
    } // if
  }
  else {
    // Initialize clog runtime
    clog_init(tags);

    clog_assert(runtime_.driver(), "you have not set the runtime driver");

    // Invoke the primary callback
    result = runtime_.driver()(argc, argv);

    clog_assert(result == 0, "non-zero return from runtime driver");
  } // if

  // Invoke registered runtime finalizations
  runtime_.finalize_runtimes(argc, argv, runtime_exit_mode_t::success);

  return result;
} // main
