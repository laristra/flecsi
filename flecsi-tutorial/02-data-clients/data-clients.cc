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

/*----------------------------------------------------------------------------*

  Simply put, a data client is a type that accesses data, i.e., it is a
  client of the FleCSI data model. The core FleCSI library provides data
  structures, e.g., the mesh topology, that can be specialized to
  provide an interface for a particular class of applications. Data
  client instances must be registered with the FleCSI runtime so that
  initialization and memory management are handled correctly. When you
  call the registration interface for a data client, you are essentially
  just declaring a variable that will be accessible through the FleCSI
  interface later during program execution.

  For the purpose of this tutorial, we have created a specialization of
  the FleCSI mesh topology and defined the type mesh_t. This mesh type
  will be used for many of the examples in the tutorial. The methods
  exposed by mesh_t are intended only to give the reader a feel for the
  kinds of interfaces that can be defined using FleCSI. A real
  specialization may expose several different mesh types and other
  specializations, and would likely have a different interface.

  The registration interface for data clients tasks the following
  arguemnts:

  (1) The data client type. This must be a valid specialization type. In
      general, application developers and end-users will not create this
      type directly. In this example, the data client type is "mesh_t".

  (2) The namespace of the data client instance that is being
      registered. Unlike task registration, this namespace does not have
      to be a valid C++ namespace. The namespace is intended to allow
      users to avoid naming collisions, i.e., internally, this argument will 
      will be used to distinguish data client instances with the same
      name from one another. In this example, the namespace is
      "clients".

  (3) The name of the client instance. This is the name of the variable
      that you would like to define. The name is arbitrary, and can be
      any legal C/C++ variable name. In this example, the name is
      "mesh".

  Below, the user should note the "ro" parameter that is used to define
  the mesh argument to the task. This parameter is specifying a privilege
  that tells the runtime what parts of the data might be modified during
  task execution. The concept of privileges will be covered in detail in
  a later example.

 *----------------------------------------------------------------------------*/

flecsi_register_data_client(mesh_t, clients, mesh);

namespace hydro {

void simple_task(mesh<ro> m) {

  // Our specialization mesh interface provides a "print" method. The
  // mechanism that allows a mesh handle to be passed to a task inherits
  // the interface of the data handle type, i.e., all of the methods of
  // the type are available to the user.

  m.print("Hello World! I am a mesh!");

} // simple_task

// Task registration is as usual...

flecsi_register_task(simple_task, hydro, loc, single);

} // namespace hydro

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  // Here, we ask the runtime for a handle to the data client variable
  // that we defined above. This handle is an opaque reference to the
  // data client variable that cannot be accessed outside of a task.
  // During task execution, this handle will be mapped into the task's
  // memory space, at which point it will be accessible by the user.

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);

  // Task execution is as usual...

  flecsi_execute_task(simple_task, hydro, single, m);

} // driver

} // namespace execution
} // namespace flecsi

/* vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : */
