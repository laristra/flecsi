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
#include <iostream>

#include<flecsi/execution/execution.h>

namespace hydro {

/*----------------------------------------------------------------------------*

  A FleCSI function is an address-space-safe function pointer that can
  be treated as data, and may be moved or mapped to a different address
  space from the one in which it was set.

  FleCSI functions are C/C++ functions. Functions that are defined in a
  header file must declared inline (This is generally true for all C++
  functions that are defined in a header file.) This example creates
  secveral functions with different return and argument types, and
  invokes them using the FleCSI function interface. There are several
  steps required to correctly define, register, and invoke a FleCSI
  function:

  (1) Define a C/C++ function and note its return and argument types.

  (2) Define a function type by calling the
      "flecsi_defnie_function_type" interface. The arguments to this
      interface are:

      (1) A type name (you invent this). In this example, the type name
          is "test_function_t". The function type name should be a
          discriptive name for the class of functions to be accessed
          through the FleCSI function interface. For example, if you
          need functions pointers to a function that computes a pressure
          from a density and an internal energy, you might call this
          type "pressure_eos_t". Similar to a function pointer, you
          could then set a FleCSI function handle to point to one of
          several function implementations that computed a pressure from
          a density and an internal energy (same function signature).

      (2) The return type of the function signature. In this example

      (3-N) The arguments to the function signature.
  
  NOTE: If you do not require a function that can be passed as data, you
  do not need to use a FleCSI function! Invoking a function through the
  FleCSI interface incurs significant overhead over a normal function
  invocation.

 *----------------------------------------------------------------------------*/

flecsi_define_function_type(void_function_t, void);
flecsi_define_function_type(pressure_eos_t, double, double, double);

void simple_function() {

  // Print message from inside of the task
  std::cout << "Hello World from " << __FUNCTION__ << std::endl;

} // simple_function

flecsi_register_function(simple_function, hydro);

double gamma(double rho, double rho_e) {
} // gamma

struct data_t {
  void_function_t test;
}; // data_t

void calling_function(data_t & d) {

  flecsi_execute_function(d.test);

} // calling_function

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  hydro::data_t d;
  d.test = flecsi_function_handle(simple_function, hydro);

  calling_function(d);
} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
