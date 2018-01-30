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

  A FleCSI task is the basic execution unit of distributed-memory (SPMD)
  and conventional task-based (MPMD) parallelism. This example
  demonstrates defining a simple task, and registering it with the
  FleCSI runtime.

  A FleCSI task is a C/C++ function. Tasks that are defined in a header
  file must either be inline or static. As you can see below, this
  example defines a task called "simple_task", and registers it with the
  runtime. The arguments to the task registration interface are:

    (1) The task name. This must be the actual name of the C/C++
        function. In this example, the task name is "simple_task".

    (2) The namespace of the task. This must be the actual namespace in
        which the task is defined. In this example, the namespace is
        "hydro".

    (3) The processor type on which the task should be executed.
        Currently, FleCSI has support for:

        loc - Latency-optimized core
        toc - Throughput-optimized core
        mpi - An MPI task

        In this example, the processor type is "loc".

    (4) The launch type. This tells the runtime whether the task should
        be launched as a member of a group of executing tasks (SPMD), or
        as a single conventional task (MPMD). These launch types
        correspond to the arguments, respectively:

        index  - Launch the task as part of a group
        single - Launch the task as a single, independent thread of
                 execution

        This nomenclature was adopted from the Legion programming
        system, and was originally developed by Nvidia. Future versions
        of FleCSI may add-to or change this. The most current version of
        the FleCSI interface can be found in the Doxygen and User Guide
        documentation.

        In this example, the launch type is single.

  The driver in this example executes the task by invoking the execution
  interface. The arguments to the execution interface are:

    (1) The task name. This has the same caveats of the registration
        interface. In this example, the task name is "simple".

    (2) The namespace of the task. This also has the same caveats of the
        registration interface. In this example, the namespace is
        "hydro".

    (3) The launch type. In this example, the launch type is "single".

  NOTES:

    - Tasks must be defined in a C++ namespace, and this namespace must
      be repeated in the registration call. This is tedious. However, it
      is a limitation of the current mechanism we have chosen to
      implement the task interface. Future versions of FleCSI will
      likely employ a DSL, which will be cleaner than the current interface.

 *----------------------------------------------------------------------------*/

void simple_task() {

  // Print message from inside of the task
  std::cout << "Hello World from " << __FUNCTION__ << std::endl;

} // simple_task

flecsi_register_task(simple_task, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // This time, the driver executes a task to do the output
  flecsi_execute_task(simple_task, hydro, single);

} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
