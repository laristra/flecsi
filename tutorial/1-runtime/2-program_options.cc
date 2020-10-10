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

/*
  Add an integer-valued command-line option with a default value of '1'. The
  option will have a long form --level, and a short form -l. The option will be
  added under the "Car Options" section.
 */

flecsi::program_option<int> trim("Car Options",
  "level,l",
  "Specify the trim level [1-10].",
  {{flecsi::option_default, 1}},
  [](flecsi::any const & v, std::stringstream & ss) {
    const int value = flecsi::option_value<int>(v);
    return value > 0 && value < 11
             ? true
             : (ss << "value(" << value << ") out-of-range") && false;
  });

/*
  Add a string-valued command-line option with a default value of "manual". The
  option will have a long form --transmission, and a short form -t. The option
  will be added under the "Car Options" section.
 */

flecsi::program_option<std::string> transmission("Car Options",
  "transmission,t",
  "Specify the transmission type [\"automatic\", \"manual\"].",
  {{flecsi::option_default, "manual"}},
  [](flecsi::any const & v, std::stringstream & ss) {
    const std::string value = flecsi::option_value<std::string>(v);
    return value == "manual" || value == "automatic"
             ? true
             : (ss << "option(" << value << ") is invalid") && false;
  });

/*
  Add an option that defines an implicit value. If the program is invoked with
  --child-seat, the value will be true. If it is invoked without --child-seat,
  the value will be false. This style of option should not be used with
  positional arguments because Boost appears to have a bug when such options
  are invoked directly before a positional option (gets confused about
  separation). We break that convention here for the sake of completeness.
 */

flecsi::program_option<bool> child_seat("Car Options",
  "child-seat,c",
  "Request a child seat.",
  {{flecsi::option_default, false}, {flecsi::option_implicit, true}});

/*
  Add a an option to a different section, i.e., "Ride Options". The enumeration
  type is not enforced by the FleCSI runtime, and is mostly for convenience.
 */

enum purpose_option : size_t { personal, business };

flecsi::program_option<size_t> purpose("Ride Options",
  "purpose,p",
  "Specify the purpose of the trip (personal=0, business=1).",
  {{flecsi::option_default, purpose_option::business}},
  [](flecsi::any const & v, std::stringstream & ss) {
    size_t value = flecsi::option_value<size_t>(v);
    return value == personal || value == business
             ? true
             : (ss << "value(" << value << ") is invalid") && false;
  });

/*
  Add an option with no default. This will allow us to demonstrate testing an
  option with has_value().
 */

flecsi::program_option<bool> lightspeed("Ride Options",
  "lightspeed",
  "Travel at the speed of light.",
  {{flecsi::option_implicit, true}, {flecsi::option_zero}});

/*
  Add a positional option. This uses a different constructor from the previous
  option declarations. Positional options are a replacement for required
  options (in the normal boost::program_options interface).
 */

flecsi::program_option<std::string> passenger_list("passenger-list",
  "The list of passengers for this trip [.txt].",
  1,
  [](flecsi::any const & v, std::stringstream & ss) {
    const std::string value = flecsi::option_value<std::string>(v);
    return value.find(".txt") != std::string::npos
             ? true
             : (ss << "file(" << value << ") has invalid suffix") && false;
  });

/*
  User-defined program options are available after FleCSI initialize has been
  invoked.
 */

int
top_level_action() {
  double price{0.0};

  /*
    Add cost for trim level. This option does not have to be checked with
    has_value() because it is defaulted. It is also unnecessary to check the
    value because it was declared with a validator function.
   */

  price += trim.value() * 100.0;

  /*
    Add cost for automatic transmission.
   */

  price += transmission.value() == "automatic" ? 200.0 : 0.0;

  /*
    Add cost for child seat.
   */

  if(child_seat.value()) {
    price += 50.0;
  }

  /*
    Deduction for business.
   */

  if(purpose.value() == business) {
    price *= 0.8;
  }

  /*
    Add cost for lightspeed. Since this option does not have a default, we need
    to check whether or not the flag was passed.
   */

  if(lightspeed.has_value()) {
    price += 1000000.0;
  }

  /*
    Do something with the positional argument.
   */

  auto read_file = [](std::string const &) {
    // Read passengers...
    return 5;
  };

  size_t passengers = read_file(passenger_list.value());

  price *= passengers * 1.10 * price;

  std::cout << "Price: $" << price << std::endl;

  return 0;
} // top_level_action

int
main(int argc, char ** argv) {

  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  } // if

  status = flecsi::start(top_level_action);

  flecsi::finalize();

  return status;
} // main
