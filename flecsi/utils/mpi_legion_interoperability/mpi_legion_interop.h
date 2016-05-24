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
#ifndef MPI_LEGION_INTEROP_HPP
#define MPI_LEGION_INTEROP_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <mutex>
#include <condition_variable>

#include "mpi.h"

#include "legion.h"
#include "realm.h"

#include "flecsi/utils/mpi_legion_interoperability/legion_handshake.h"
#include "flecsi/utils/mpi_legion_interoperability/mpi_legion_data.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

// MPILegionInterp class

 //variadic templates for dataTypes for data to be shared
class MPILegionInterop {

  public:
  MPILegionInterop(){handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);};
  ~MPILegionInterop();

  //variadic arguments for data to be shared
  template <typename... CommonDataTypes>
  void copy_data_and_handoff_to_legion (CommonDataTypes&&... CommData);

  void legion_configure();

  void connect_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime);

  void handoff_to_legion(void);

  void wait_on_legion(void);

  int handoff_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime);

//toimplement
  template <typename... CommonDataTypes>
  void copy_data_and_handoff_to_mpi (CommonDataTypes&&... CommData); 

  void wait_on_mpi(void);

//  CommonDataType CommonData;
  ExtLegionHandshake *handshake;
};

 template <typename T>
 void assign_legion_to_mpi(T &data){
  data.legion_accessor = data.mpi_ptr();
 }

 template <typename... CommonDataTypes>
 void MPILegionInterop::copy_data_and_handoff_to_legion(CommonDataTypes&&... CommData) 
{
   assign_legion_to_mpi(CommData...) ;
//   CommData.legion_object = CommData.mpi_object.get_ptr();
   handshake->ext_handoff_to_legion();
  }

 void MPILegionInterop::legion_configure()
 {
    handshake->ext_init();
 }

 void MPILegionInterop::connect_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
 {
    handshake->legion_init();
 }

 void MPILegionInterop::handoff_to_legion(void)
 {
   handshake->ext_handoff_to_legion();
 }

 void MPILegionInterop::wait_on_legion(void)
 {
   handshake->ext_wait_on_legion();
 }

 int MPILegionInterop::handoff_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
 {
  handshake->legion_handoff_to_ext();
  return 0;
 }

 void MPILegionInterop::wait_on_mpi(void)
 {
  handshake->legion_wait_on_ext();
 }


#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

