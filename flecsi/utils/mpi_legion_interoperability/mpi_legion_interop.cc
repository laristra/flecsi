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

#include "mpi_legion_interop.h"

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




/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
