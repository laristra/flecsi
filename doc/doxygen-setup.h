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

#ifndef FLECSI_DOXYGEN_SETUP_INCLUDE_ERROR
#error "This file is only provided for Doxygen setup. DO NOT INCLUDE IT!!!"

//----------------------------------------------------------------------------//
// Control Model
//----------------------------------------------------------------------------//

/**
 * Define the Control Model group.
 * @defgroup control Control Interface
 **/

//----------------------------------------------------------------------------//
// Execution Model
//----------------------------------------------------------------------------//

/**
 * The FleCSI execution model is an hierarchically parallel abstraction that
 * divides work into coarse-grained, distributed-memory tasks, and
 * fine-grained data-parallel kernels. Tasks are functionally pure with
 * controlled side effects. Kernels are shared-memory and may exploit
 * varying degrees of memory consistency, e.g., sequential or relaxed.
 *
 * @defgroup execution Execution Interface
 **/

/**
 * This module contains the Legion backend implementation of the FleCSI
 * execution model.
 *
 * @defgroup legion-execution Legion Execution Backend
 * @ingroup execution
 **/

/**
 * This module contains the serial backend implementation of the FleCSI
 * execution model.
 *
 * @defgroup serial-execution Serial Execution Backend
 * @ingroup execution
 **/

//----------------------------------------------------------------------------//
// Runtime Model
//----------------------------------------------------------------------------//

/**
 * The FleCSI runtime model maintains global context.
 *
 * \defgroup runtime Runtime Interface
 **/

/**
 * This module contains the Legion backend implementation of the FleCSI
 * runtime model.
 *
 * \defgroup legion-runtime Legion Runtime Backend
 * \ingroup runtime
 **/

//----------------------------------------------------------------------------//
// Data Model
//----------------------------------------------------------------------------//

/**
 * Define the Data Model group.
 * @defgroup data Data Interface
 **/

/**
 * Define the Legion Data Backend group.
 * @defgroup legion-data Legion Data Backend
 * @ingroup data
 **/

/**
 * Define the Serial Data Backend group.
 * @defgroup serial-data Serial Data Backend
 * @ingroup data
 **/

//----------------------------------------------------------------------------//
// Topology
//----------------------------------------------------------------------------//

/**
 * Define the Topology group.
 * @defgroup topology Topology Interface
 **/

/**
 * Define the Mesh Topology group.
 * @defgroup mesh-topology Mesh Toplogy Interface
 * @ingroup topology
 **/

/**
 * Define the Structured Mesh Topology group.
 * @defgroup structured-mesh-topology Structured Mesh Toplogy Interface
 * @ingroup topology
 **/

/**
 * Define the Tree Topology group.
 * @defgroup tree-topology Tree Toplogy Interface
 * @ingroup topology
 **/

//----------------------------------------------------------------------------//
// Utilities
//----------------------------------------------------------------------------//

/**
 * Define the utilities group.
 * @defgroup utils Utilities Interface
 **/

//----------------------------------------------------------------------------//
// FleCSI Tutorial
//----------------------------------------------------------------------------//

/**
 * Define the FleCSI Tutorial group.
 * @defgroup tutorial FleCSI Tutorial Examples
 **/

//----------------------------------------------------------------------------//
// Flog
//----------------------------------------------------------------------------//

/**
 * The FleCSI logging utility (flog) provides a C++ interface for capturing
 * output during program execution.
 * @defgroup flog FleCSI Logging Interface (flog)
 **/
