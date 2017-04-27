/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef FLECSI_DOXYGEN_SETUP_INCLUDE_ERROR
#error "This file is only provided for Doxygen setup. DO NOT INCLUDE IT!!!"

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
 * Define the Graph Topology group.
 * @defgroup graph-topology Graph Toplogy Interface
 * @ingroup topology
 **/

/**
 * Define the Structured Graph Topology group.
 * @defgroup structured-graph-topology Structured Graph Toplogy Interface
 * @ingroup topology
 **/

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
