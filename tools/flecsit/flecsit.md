<!-- CINCHDOC DOCUMENT(User Guide) SECTION(FleCSIT) -->

# FleCSIT Tool

The *flecsit* command line tool allows the user to compile FleCSI task
drivers into executable programs. It is intended as a sandbox utility
for application and methods developers to experiment with the FleCSI
programming system without having to cope with the full complexity of a
production application. That said, code developed with the FleCSIT tool
is fully compatible with FleCSI and can be directly integrated into
full application projects. This approach is consistent with the
FLeCSI philosophy of allowing scalable development from the laptop to
large-scale supercomputing systems.

In addition to providing an experimental capability, FleCSIT also
provides static analysis features that make using the FleCSI programming
system easier. In particular, FleCSIT provides compile-time checking of
some of the FleCSI data, execution and control model interfaces. An
example motivation for this feature is the fact that FleCSI data
registration does not actually occur until runtime. This means that
simple programmer errors cannot be caught by the compiler. The FleCSIT
tool provides these basic checks so that errors can be detected and
corrected without going through a complete build and run cycle.

FleCSIT is a service-based python program that allows easy extensibility
to provide new features as they become necessary, and we will continue
to develope this tool with new capabilities to ease development with
FleCSI.

Currently, the following services are available in FleCSIT:

* **compile** This service does compilation and compile-time checking of
  a user-defined input driver.
* **analyze** This service provides static analysis of user code. It is
  not intended as an end user tool and requires expert knowledge of the
  FleCSI programming system.

The *flecsit* executable provides additional help information from the
command line

    % flecsit service --help

where *service* is the name of one of the FleCSIT services. Help for the
*flecsit* executable itself is available with

    % flecsit --help

## Compile Service

Here we provide a brief example of how to use the FleCSIT compile
service. To begin, create a file named *driver.h* with the following
contents:

    #ifndef driver_h
    #define driver_h

	#incldue <iostream>

	void driver(int argc, char ** argv) {
	  std::cout << "Hello World" << std::endl;
	} // driver

    #endif // driver_h

To compile this file using the *flecsit* command, type

    % flecsit compile driver.h

This will create an executable called *driver.arch*, where *arch* is the
name of the architecture for which FleCSI was compiled.

## Docker Containers

The FleCSI and FleCSI Specializations projects provide pre-built docker
containers for experimental development using the FleCSI programming
system. To get the latest build, pull either the FleCSI or FleCSI
Specialziations containers

    % docker pull flecsi/flecsi-sp:fedora_serial

(Other os versions and runtimes are availble in addition to serial.) To
run the container, type

    % docker run -it flecsi/flecsi-sp:fedora_serial /bin/bash

This will open a bash shell on the container allowing access to the
pre-built FleCSI environment.

[Docker](https://www.docker.com) is a software containerization
platform. Instructions for installing and using Docker are available on
their website.

<!-- vim: set fo=cqt tw=72 : -->
