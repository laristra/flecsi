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

#include <flecsi/execution.hh>
#include <flecsi/utils/flog.hh>

int top_level_action(int, char **) {
  std::cout << "Hello World" << std::endl;
  return 0;
} // top_level_action

inline bool top_level_action_registered =
  flecsi::runtime::context_t::instance().register_top_level_action(
  top_level_action);

/*
  Add an optional command-line option with a default value of '1'. The option
  will have a long form --level, and a short form -l. The option will be added
  under the "Example Options" section.
 */

int optimization_level;
auto registered_opt = flecsi::add_program_option("Example Options",
  "level,l",
  boost::program_options::value(&optimization_level)->default_value(1),
  "Specify the optimization level.");

/*
  Add a required option.
 */

auto registered_req = flecsi::add_program_option("Example Options",
  "required,r",
  boost::program_options::value<int>()->required(),
  "Specify a required options.");

/*
  User-defined program options are available after FleCSI initialize has been
  invoked.
 */

int main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::runtime::status::success) {
    return status == flecsi::runtime::status::help ? 0 : status;
  } // if

  /*
    The FleCSI initialize method handles command-line parsing and error
    checking. It also provides a help option that will print a usage message.
    This statement gets a map of the user-specified command-line options and
    their values (where defined).
   */

  auto vm = flecsi::program_options_variables_map();

  /*
    Print the optimization level that was passed as a reference to Boost.
   */

  std::cout << "Optimization level: " << optimization_level << std::endl;

  /*
    Print the required value.
   */

  std::cout << "Required value: " << vm["required"].as<int>() << std::endl;

  status = flecsi::start();

  flecsi::finalize();

  return status;
} // main
