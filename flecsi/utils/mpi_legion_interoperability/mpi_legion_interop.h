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

//WARNING!!!!!!!!!!!!!!!!!!
//WARNING!!!!!!!!!!!!!!!!!!!
//
//this header file should not be explicitly included in flecsi code to avoi 
//sircular dependency
//to ensure this, I ude following "define" guard"
#ifdef MPI_LEGION_INTEROP_HPP_INCLUDED_IN_EXECUTION_POLICY_H

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

namespace flecsi
{
namespace mpilegion
{
class MPILegionInterop;
static MPILegionInterop *MPILegionInteropHelper;
 
// MPILegionInterp class
class MPILegionInterop {

  public:
  MPILegionInterop(){handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);};
  ~MPILegionInterop(){ delete handshake;};

  //variadic arguments for data to be shared
  template <typename... CommonDataTypes>
  void copy_data_from_mpi_to_legion (CommonDataTypes&&... CommData, 
                  context_t<mpilegion_execution_policy_t>  &ctx);

  //copy MpiLegionStorage data from mpi to legion
  void copy_data_from_mpi_to_legion (context_t<mpilegion_execution_policy_t>  &ctx);

  void legion_configure();

  static void connect_to_mpi_task (const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                      LegionRuntime::HighLevel::Context ctx, 
                      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  void handoff_to_legion(void);

  void wait_on_legion(void);

  static void  handoff_to_mpi_task (const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                      LegionRuntime::HighLevel::Context ctx, 
                      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  template <typename... CommonDataTypes>
  void copy_data_from_legion_to_mpi (CommonDataTypes&&... CommData,
                             context_t<mpilegion_execution_policy_t>  &ctx); 

  void copy_data_from_legion_to_mpi (context_t<mpilegion_execution_policy_t>  &ctx);

  static void wait_on_mpi(const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                      LegionRuntime::HighLevel::Context ctx, 
                      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  template <typename Type, uint64_t value>
  void add_array_to_storage(flecsi::mpilegion::MPILegionArray<Type, value> *A);

  uint storage_size(void);

  void allocate_legion(context_t<mpilegion_execution_policy_t>  &ctx);
  void legion_init(context_t<mpilegion_execution_policy_t>  &ctx);
  void mpi_init (void);

  static void register_tasks(void);

  void calculate_number_of_pocs (void);

  void connect_with_mpi(context_t<mpilegion_execution_policy_t>  &ctx);

  void handoff_to_mpi(context_t<mpilegion_execution_policy_t>  &ctx);
 
 public:
//  CommonDataType CommonData;
  ExtLegionHandshake *handshake=nullptr;

  Rect<2> all_processes;
  Rect<1> local_procs;

  std::vector <std::shared_ptr<MPILegionArrayStorage_t>> MpiLegionStorage;
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
  assert (MpiLegionStorage.size()!=0);
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
 assert (MpiLegionStorage.size()!=0);
 for (uint i=0; i<MpiLegionStorage.size(); i++)
  MpiLegionStorage[i]->copy_legion_to_mpi(ctx);
 }

 void MPILegionInterop::legion_configure()
 {
    MPILegionInteropHelper->handshake->ext_init();
 }

 void MPILegionInterop::connect_to_mpi_task (const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                     LegionRuntime::HighLevel::Context ctx, 
                     LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
std::cout <<"inside connect_to_mpi"<<std::endl;
   MPILegionInteropHelper->handshake->legion_init();
 }

 void MPILegionInterop::handoff_to_legion(void)
 {
   MPILegionInteropHelper->handshake->ext_handoff_to_legion();
 }

 void MPILegionInterop::wait_on_legion(void)
 {
   MPILegionInteropHelper->handshake->ext_wait_on_legion();
 }

 void MPILegionInterop::handoff_to_mpi_task (const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                      LegionRuntime::HighLevel::Context ctx, 
                      LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
   MPILegionInteropHelper->handshake->legion_handoff_to_ext();
 }

 void MPILegionInterop::wait_on_mpi(const Legion::Task *legiontask,
                      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
                      LegionRuntime::HighLevel::Context ctx, 
                      LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
  MPILegionInteropHelper->handshake->legion_wait_on_ext();
 }


 template <typename Type, uint64_t value>
 void MPILegionInterop::add_array_to_storage(MPILegionArray<Type, value> *A)
 {
   MpiLegionStorage.push_back(std::shared_ptr<MPILegionArrayStorage_t>(A)); 
 }

 uint MPILegionInterop::storage_size(void)
 {
   return MpiLegionStorage.size();
 }


 void MPILegionInterop::allocate_legion(context_t<mpilegion_execution_policy_t>  &ctx)
 {
   assert (MpiLegionStorage.size()!=0);
   for (uint i=0; i<MpiLegionStorage.size(); i++)
     MpiLegionStorage[i]->allocate_legion(ctx);
 }

 void MPILegionInterop::legion_init(context_t<mpilegion_execution_policy_t>  &ctx)
 {
  assert (MpiLegionStorage.size()!=0);
  for (uint i=0; i<MpiLegionStorage.size(); i++)
    MpiLegionStorage[i]->legion_init(ctx);
 }
 
 void MPILegionInterop::mpi_init (void)
 {
   assert (MpiLegionStorage.size()!=0);
   for (uint i=0; i<MpiLegionStorage.size(); i++)
     MpiLegionStorage[i]->mpi_init();
 }

 //static:
 void MPILegionInterop::register_tasks(void)
 {
  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
                         <connect_to_mpi_task>( CONNECT_MPI_TASK_ID,
                          LegionRuntime::HighLevel::Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, 
                          LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
                          "connect_to_mpi_task");

   LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
                           <handoff_to_mpi_task>( HANDOFF_TO_MPI_TASK_ID,
                           LegionRuntime::HighLevel::Processor::LOC_PROC, false/*single*/, true/*index*/,
                           AUTO_GENERATE_ID, 
                           LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
                           "handoff_to_mpi_task");
 
   LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
                          <wait_on_mpi>( WAIT_ON_MPI_TASK_ID,
                          LegionRuntime::HighLevel::Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, 
                          LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
                          "wait_on_mpi_task");

 }


 void MPILegionInterop::calculate_number_of_pocs(void)
 {
  int num_local_procs=0;
#ifndef SHARED_LOWLEVEL
  // Only the shared lowlevel runtime needs to iterate over all points
  // on each processor.
  int num_points = 1;
  int num_procs = 0;
  {
   std::set<LegionRuntime::HighLevel::Processor> all_procs;
   Realm::Machine::get_machine().get_all_processors(all_procs);
   for(std::set<LegionRuntime::HighLevel::Processor>::const_iterator it = all_procs.begin();
      it != all_procs.end();
      it++){
    if((*it).kind() == LegionRuntime::HighLevel::Processor::LOC_PROC)
      num_procs++;
   }
  }
  num_local_procs=num_procs;
#else
  int num_procs = LegionRuntime::HighLevel::Machine::get_machine()->get_all_processors().size();
  int num_points = rank->proc_grid_size.x[0] * rank->proc_grid_size.x[1] * rank->proc_grid_size.x[2];
#endif
  printf("Attempting to connect %d processors with %d points per processor\n",
         num_procs, num_points);
  LegionRuntime::Arrays::Point<2> all_procs_lo, all_procs_hi;
  all_procs_lo.x[0] = all_procs_lo.x[1] = 0;
  all_procs_hi.x[0] = num_procs - 1;
  all_procs_hi.x[1] = num_points - 1;
  this->all_processes =  LegionRuntime::Arrays::Rect<2>(all_procs_lo, all_procs_hi);
  this->local_procs = LegionRuntime::Arrays::Rect<1>(0,num_local_procs);
 }

 void MPILegionInterop::connect_with_mpi(context_t<mpilegion_execution_policy_t>  &ctx)
 {
   calculate_number_of_pocs();
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher connect_mpi_launcher(CONNECT_MPI_TASK_ID,
                                       LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes),
                                       LegionRuntime::HighLevel::TaskArgument(0, 0),
                                       arg_map);
  //run legion_init() from each thead
  LegionRuntime::HighLevel::FutureMap fm1 =
         ctx.runtime()->execute_index_space(ctx.legion_ctx(), connect_mpi_launcher);

  //run some legion task here
  fm1.wait_all_results();
 
 }

 void MPILegionInterop::handoff_to_mpi(context_t<mpilegion_execution_policy_t>  &ctx)
 {
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher handoff_to_mpi_launcher(HANDOFF_TO_MPI_TASK_ID,
                                   LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes),
                                   LegionRuntime::HighLevel::TaskArgument(0, 0),
                                   arg_map);
    ctx.runtime()->execute_index_space(ctx.legion_ctx(), handoff_to_mpi_launcher);
 }

}//end namespace mpilegion

} //end namespace flecsi

#endif

#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

