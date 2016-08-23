/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef TASK_IDS
#define TASK_IDS

/*!
* \file mpilegion/task_ids.h
* \authors demeshko
* \date Initial file creation: Jul 2016
*
*  HelperTaskIDs enum struct includes TASK_IDs for the legion tasks used
*  in mpi_legion_interop_t class
*  These needs to be global due to the fact that the same IDs are used in
*  mapper.h to se up special rools on executing the tasks used to switch
*  between MPI and Legion rantimes
*/

enum HelperTaskIDs{
 CONNECT_MPI_TASK_ID    = 0x00099997,
 HANDOFF_TO_MPI_TASK_ID = 0x00099998,
 WAIT_ON_MPI_TASK_ID    = 0x00099999,
};

#endif

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

