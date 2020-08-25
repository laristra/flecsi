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
#include <flecsi/flog.hh>

#include <realm/logging.h>

#include "control.hh"

using namespace flecsi;

/*
  Trivial task (no arguments, no return).
 */

void
trivial() {
  flog(info) << "Hello World" << std::endl;
}

/*
  Task with return value.
 */

int
with_return() {
  int value{100};
  flog(info) << "Returning value " << value << std::endl;
  return value;
}

/*
  Task with by-value argument.
 */

int
with_by_value_argument(std::vector<size_t> v) {
  std::stringstream ss;
  int retval{0};
  ss << "Parameter values: ";
  for(auto i : v) {
    retval += i;
    ss << i << " ";
  } // for
  flog(info) << ss.str() << std::endl;

  return retval;
}

/*
  Templated task.
 */

template<typename Type>
Type
templated(Type t) {
  Type retval{t + Type(10)};
  flog(info) << "Returning value " << retval << " with type "
             << typeid(t).name() << std::endl;
  return retval;
}

int
advance() {

  /*
    Execute the trivial task.
   */

  execute<trivial>();

  /*
    Execute a task with a return value.
   */

  {

    /*
      A future is a mechanism to access the result of an asynchronous operation.
     */

    auto future = execute<with_return>();

    /*
      The 'wait()' method waits for the result to become available.
     */

    future.wait();

    /*
      The 'get()' method returns the result. Note that calling 'get()' by itself
      will wait for the result to become available. The call to 'wait()' in this
      example are illustrative.
     */

    flog(info) << "Got value " << future.get() << std::endl;
  } // scope

  /*
    Execute a task that takes an argument by-value. FleCSI tasks can take any
    valid C++ type by value. However, because task data must be relocatable,
    you cannot pass pointer arguments, or arguments that contain pointers.
   */

  {
    std::vector<size_t> v = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    auto future = execute<with_by_value_argument>(v);
    flog(info) << "Sum is " << future.get() << std::endl;
  }

  /*
    Execute a templated task.
   */

  {
    double value{32.0};
    auto future = execute<templated<double>>(value);
    flog(info) << "Got templated value " << future.get() << std::endl;
  }

  return 0;
}
control::action<advance, cp::advance> advance_action;
