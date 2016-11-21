/*~-------------------------------------------------------------------------~~*
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
 *~-------------------------------------------------------------------------~~*/
#include <iostream>
//#include "flecsi.h" 
/*#include "flecsi/execution/mpilegion_execution_policy.h"

//using execution_type = flecsi::execution_t<flecsi::mpilegion_execution_policy_t>;
using return_type_t = 
   flecsi::execution_t<flecsi::mpilegion_execution_policy_t>::return_type_t;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

typedef typename flecsi::context_t<flecsi::mpilegion_execution_policy_t> mpilegion_context;
namespace flecsi
{
 void mpilegion_top_level_task(mpilegion_context &&ctx,int argc, char** argv)
 {
   std::cout<<"insidr TLT" <<std::endl;
   flecsi::mpilegion::MPILegionInteropHelper->connect_with_mpi(ctx);

   std::cout<<"handshake is connected to Legion" <<std::endl;
   std::cout<<"some computations in Legion"<<std::endl;
 
   flecsi::mpilegion::MPILegionInteropHelper->handoff_to_mpi(ctx);
 }
}

int driver(int argc, char ** argv) {
 std::cout<< "inside sprint1"<<std::endl; 

  flecsi::execution_t<flecsi::mpilegion_execution_policy_t>::execute_driver(
     flecsi::mpilegion_top_level_task,argc,argv);

  flecsi::mpilegion::MPILegionInteropHelper->legion_configure();

  std::cout<<"back in MPI, do something here"<<std::endl;
 
  flecsi::mpilegion::MPILegionInteropHelper->handoff_to_legion();

  //legion is running
 
  flecsi::mpilegion::MPILegionInteropHelper->wait_on_legion();

 std::cout<<"we are back to MPI to call MPI_Finalize()"<<std::endl;

  return 0;
} 
*/
/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
