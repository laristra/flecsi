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

#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mapper.h"
#include "flecsi/execution/mpilegion/task_ids.h"
//#include "flecsi/execution/context.h"
namespace flecsi{
namespace execution{


// MPILegionInterp class
class MPILegionInterop {

  public:
  MPILegionInterop() {};
  ~MPILegionInterop(){};

  private:
   MPILegionInterop(const MPILegionInterop&);
   MPILegionInterop& operator=(const MPILegionInterop&);


  public:
   void initialize(void)
   {
    register_tasks(); 
    LegionRuntime::HighLevel::HighLevelRuntime::set_registration_callback(
      mapper_registration);
   }
 
  //variadic arguments for data to be shared
  template <typename... CommonDataTypes>
  void copy_data_from_mpi_to_legion (CommonDataTypes&&... CommData, 
              LegionRuntime::HighLevel::Context ctx,
              LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
  void legion_configure(){ 
     ExtLegionHandshake::instance().ext_init();
     }

  static void connect_to_mpi_task (
      const Legion::Task *legiontask,
      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
      LegionRuntime::HighLevel::Context ctx, 
      LegionRuntime::HighLevel::HighLevelRuntime *runtime)
      {
         std::cout <<"inside connect_to_mpi"<<std::endl;
         ExtLegionHandshake::instance().legion_init();
      }

  void handoff_to_legion(void){
    ExtLegionHandshake::instance().ext_handoff_to_legion();
  }

  void wait_on_legion(void){
    ExtLegionHandshake::instance().ext_wait_on_legion();
  }

  static void  handoff_to_mpi_task (
      const Legion::Task *legiontask,
      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
      LegionRuntime::HighLevel::Context ctx, 
      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  template <typename... CommonDataTypes>
  void copy_data_from_legion_to_mpi (CommonDataTypes&&... CommData,
          LegionRuntime::HighLevel::Context ctx,
          LegionRuntime::HighLevel::HighLevelRuntime *runtime); 

  static void wait_on_mpi_task(
      const Legion::Task *legiontask,
      const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
      LegionRuntime::HighLevel::Context ctx, 
      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  static void register_tasks(void);

  void calculate_number_of_procs (void);

  void connect_with_mpi(LegionRuntime::HighLevel::Context ctx,
      LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  void handoff_to_mpi(LegionRuntime::HighLevel::Context ctx,
      LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
  void wait_on_mpi(LegionRuntime::HighLevel::Context ctx,
      LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
 public:
//  CommonDataType CommonData;

  std::function<void()> shared_func;

  bool call_mpi=false;

  Rect<2> all_processes;
  Rect<1> local_procs;

};


 template <typename... CommonDataTypes>
inline void MPILegionInterop::copy_data_from_mpi_to_legion(
    CommonDataTypes&&... CommData, 
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime)
    {
    }


 template <typename... CommonDataTypes>
inline void MPILegionInterop::copy_data_from_legion_to_mpi(
     CommonDataTypes&&... CommData, 
     LegionRuntime::HighLevel::Context ctx,
     LegionRuntime::HighLevel::HighLevelRuntime *runtime)  
   {  
     }


inline void MPILegionInterop::handoff_to_mpi_task (
     const Legion::Task *legiontask,
     const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
     LegionRuntime::HighLevel::Context ctx, 
     LegionRuntime::HighLevel::HighLevelRuntime *runtime)
     {
       ExtLegionHandshake::instance().legion_handoff_to_ext();
      }

inline void MPILegionInterop::wait_on_mpi_task(
    const Legion::Task *legiontask,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
    LegionRuntime::HighLevel::Context ctx, 
    LegionRuntime::HighLevel::HighLevelRuntime *runtime)
    {
     ExtLegionHandshake::instance().legion_wait_on_ext();
    }

 //static:
inline void MPILegionInterop::register_tasks(void)
 {
  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <connect_to_mpi_task>( 
        CONNECT_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC, 
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "connect_to_mpi_task");

  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <handoff_to_mpi_task>( 
        HANDOFF_TO_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC,
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "handoff_to_mpi_task");
 
  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <wait_on_mpi_task>( 
        WAIT_ON_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC, 
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "wait_on_mpi_task");

 }


inline void MPILegionInterop::calculate_number_of_procs(void)
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
   for(std::set<LegionRuntime::HighLevel::Processor>::const_iterator 
      it = all_procs.begin();
      it != all_procs.end();
      it++){
            if((*it).kind() == LegionRuntime::HighLevel::Processor::LOC_PROC)
               num_procs++;
            }
  }
  num_local_procs=num_procs;
#else
  int num_procs = LegionRuntime::HighLevel::Machine::get_machine()->
                                            get_all_processors().size();
  int num_points = rank->proc_grid_size.x[0] * rank->proc_grid_size.x[1] 
                                            * rank->proc_grid_size.x[2];
#endif
  printf("Attempting to connect %d processors with %d points per processor\n",
         num_procs, num_points);
  LegionRuntime::Arrays::Point<2> all_procs_lo, all_procs_hi;
  all_procs_lo.x[0] = all_procs_lo.x[1] = 0;
  all_procs_hi.x[0] = num_procs - 1;
  all_procs_hi.x[1] = num_points - 1;
  this->all_processes =  LegionRuntime::Arrays::Rect<2>(all_procs_lo,
                                                        all_procs_hi);
  this->local_procs = LegionRuntime::Arrays::Rect<1>(0,num_local_procs);
 }

inline void MPILegionInterop::connect_with_mpi(
     LegionRuntime::HighLevel::Context ctx,
     LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
   calculate_number_of_procs();
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher connect_mpi_launcher(
         CONNECT_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);

  //run legion_init() from each thead
  LegionRuntime::HighLevel::FutureMap fm1 =
     runtime->execute_index_space(ctx, connect_mpi_launcher);

  //run some legion task here
  fm1.wait_all_results();
 
 }

inline void MPILegionInterop::handoff_to_mpi(
     LegionRuntime::HighLevel::Context ctx,
     LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher handoff_to_mpi_launcher(
         HANDOFF_TO_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);
   runtime->execute_index_space( ctx, handoff_to_mpi_launcher);
 }

inline void MPILegionInterop::wait_on_mpi(
     LegionRuntime::HighLevel::Context ctx,
     LegionRuntime::HighLevel::HighLevelRuntime *runtime)
 {
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher wait_on_mpi_launcher(
         HANDOFF_TO_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);
   runtime->execute_index_space(ctx,wait_on_mpi_launcher);
 }

//  MPILegionInterop* MPILegionInterop::interop_obj_=0;
} //end namespace execution
} //end namespace flecsi


#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

