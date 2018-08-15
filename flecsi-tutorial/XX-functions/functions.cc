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

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, hydro, pressure, double, dense, 1, cells);

namespace example {

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

  (3) ...
  
  NOTE: If you do not require a function that can be passed as data, you
  do not need to use a FleCSI function! Invoking a function through the
  FleCSI interface incurs significant overhead over a normal function
  invocation.

 *----------------------------------------------------------------------------*/

#define PRINT_MESSAGE(s) \
  std::cout << "Executing " << s << std::endl;

// Define a function type for a very simple function.
// NOTE: For functions that do not take any arguments, it is only
// necessary to specify the return type. In this example, the return
// type is 'void'.

flecsi_define_function_type(simple_function_t, void);

void simple_function() {

  PRINT_MESSAGE(__FUNCTION__);

} // simple_function

flecsi_register_function(simple_function, example);

//---

flecsi_define_function_type(argument_function_t, int, double);

int argument_function(double arg) {
  PRINT_MESSAGE(__FUNCTION__ << " with value " << arg);
  return 0;
} // arugment_function

flecsi_register_function(argument_function, example);

int argument_task(mesh<ro> m, field<ro> p, argument_function_t fh) {

  for(auto c: m.cells()) {
    if(flecsi_execute_function(fh, p(c))) {
      return 1;
    } // if
  } // for

  return 0;
} // argument_task

flecsi_register_task(argument_task, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  {
  // Get a handle to the simple function
  auto fh = flecsi_function_handle(simple_function, example);

  // Invoke the simple function
  flecsi_execute_function(fh);
  } // simple function scope

  {
  auto fh = flecsi_function_handle(argument_function, example);
  int retval = flecsi_execute_function(fh, 5.0);
  } // arguemnt function scope

  {
  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto fh = flecsi_function_handle(argument_function, example);
  auto p = flecsi_get_handle(m, hydro, pressure, double, dense, 0);
  auto retval = flecsi_execute_task(argument_task, example, single, m, p, fh);
  } // argument function scope

} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
