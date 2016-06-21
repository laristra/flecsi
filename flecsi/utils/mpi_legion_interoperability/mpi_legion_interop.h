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
#include "flecsi/utils/mpi_legion_interoperability/mapper.h"
#include "flecsi/utils/mpi_legion_interoperability/task_ids.h"
#include "flecsi/execution/mpilegion_execution_policy.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;
using namespace flecsi::mpilegion;
//using namespace flecsi;


namespace flecsi
{
namespace mpilegion
{

// MPILegionInterp class

 //variadic templates for dataTypes for data to be shared
class MPILegionInterop {

  public:
  MPILegionInterop(){handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);};
  ~MPILegionInterop(){};

  //variadic arguments for data to be shared
  template <typename... CommonDataTypes>
  void copy_data_from_mpi_to_legion (CommonDataTypes&&... CommData, 
                  context_t<mpilegion_execution_policy_t>  &ctx);

  //copy MpiLegionStorage data from mpi to legion
  void copy_data_from_mpi_to_legion (context_t<mpilegion_execution_policy_t>  &ctx);

  void legion_configure();

  void connect_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      context_t<mpilegion_execution_policy_t>  &ctx);

  void handoff_to_legion(void);

  void wait_on_legion(void);

  int handoff_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      context_t<mpilegion_execution_policy_t>  &ctx);

//toimplement
  template <typename... CommonDataTypes>
  void copy_data_from_legion_to_mpi (CommonDataTypes&&... CommData,
                             context_t<mpilegion_execution_policy_t>  &ctx); 

  void copy_data_from_legion_to_mpi (context_t<mpilegion_execution_policy_t>  &ctx);

  void wait_on_mpi(const Task *task,
                        const std::vector<PhysicalRegion> &regions,
                        context_t<mpilegion_execution_policy_t>  &ctx);

 public:
//  CommonDataType CommonData;
  ExtLegionHandshake *handshake;

  std::vector <std::shared_ptr<flecsi::mpilegion::MPILegionArrayStorage_t>> MpiLegionStorage;
//  std::map<std::string,typename MPILegionArray> MPILegionArrays; //creates a map between the array's name and Array itself

  static MPILegionInterop* get_interop_object(const Point<3> &pt, bool must_match);  
#ifndef SHARED_LOWLEVEL
  static MPILegionInterop*& get_local_interop_object(void);
#else
  static std::map<Point<3>, MPILegionInterop*, Point<3>::STLComparator>& get_interop_objects(void);
 // static pthread_mutex_t& get_local_mutex(void);
#endif

};

 template <typename Type>
 void copy_mpi_to_legion(Type &a, context_t<mpilegion_execution_policy_t>  &ctx)
 {
   a.copy_mpi_to_legion(ctx);
 }

 template <typename... CommonDataTypes>
 void MPILegionInterop::copy_data_from_mpi_to_legion(CommonDataTypes&&... CommData, 
                         context_t<mpilegion_execution_policy_t>  &ctx) 
 {
   copy_mpi_to_legion(CommData..., ctx);
 }

 void MPILegionInterop::copy_data_from_mpi_to_legion(context_t<mpilegion_execution_policy_t>  &ctx)  
 {  
  assert (MpiLegionStorage.size()==0);
 for (uint i=0; i<MpiLegionStorage.size(); i++)
  MpiLegionStorage[i]->copy_mpi_to_legion(ctx);
 }


 template <typename Type>
 void copy_legion_to_mpi(Type &a, context_t<mpilegion_execution_policy_t>  &ctx)
 {
   a.copy_legion_to_mpi(ctx);
 }

 template <typename... CommonDataTypes>
 void MPILegionInterop::copy_data_from_legion_to_mpi(CommonDataTypes&&... CommData, 
                         context_t<mpilegion_execution_policy_t>  &ctx)  
 {  
//   CommData.copy_legion_to_mpi(ctx)...;
   copy_legion_to_mpi(CommData..., ctx);
 }
 void MPILegionInterop::copy_data_from_legion_to_mpi(context_t<mpilegion_execution_policy_t>  &ctx)            
 {
 assert (MpiLegionStorage.size()==0);
 for (uint i=0; i<MpiLegionStorage.size(); i++)
  MpiLegionStorage[i]->copy_legion_to_mpi(ctx);
 }

 void MPILegionInterop::legion_configure()
 {
    this->handshake->ext_init();
 }

 void MPILegionInterop::connect_to_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      context_t<mpilegion_execution_policy_t>  &ctx)
 {
    this->handshake->legion_init();
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
                      context_t<mpilegion_execution_policy_t>  &ctx)
 {
  handshake->legion_handoff_to_ext();
  return 0;
 }

 void MPILegionInterop::wait_on_mpi(const Task *task,
                        const std::vector<PhysicalRegion> &regions,
                        context_t<mpilegion_execution_policy_t>  &ctx)
 {
  handshake->legion_wait_on_ext();
 }


}//end namespace mpilegion

} //end namespace flecsi

#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

