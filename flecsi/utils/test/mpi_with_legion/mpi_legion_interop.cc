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

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>
#include <type_traits> // std::is_same

// user includes

//do not include mpi_legion_interop.h explicitly, it is included in 
//mpilegion_execution_policy.h to avoid circular dependency
//#include "flecsi/utils/mpi_legion_interoperability/mpi_legion_interop.h"
#include "flecsi/execution/mpilegion_execution_policy.h"
#include "flecsi/execution/task.h"

using namespace flecsi;

using execution_type = flecsi::execution_t<flecsi::mpilegion_execution_policy_t>;
using return_type_t = execution_type::return_type_t;

enum TaskIDs{
 HELLOWORLD_TASK_ID        =0x00000100,
 SPMD_INIT_TID             =0x00000200,
};


return_type_t world_size() {
#ifdef DEBUG
  printf ("inside MPI function \n");
#endif 
  int world_size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  return 0;
}

//make Array global only for the simple test example
//in general, we are not suppose to do so if the object
//is used in Legion

const int nElements=10;
MPILegionArray<double, nElements> Array;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

/* ------------------------------------------------------------------------- */
typedef typename flecsi::context_t<flecsi::mpilegion_execution_policy_t> mpilegion_context;
namespace flecsi
{
void mpilegion_top_level_task(mpilegion_context &&ctx,int argc, char** argv)
{
  int num_local_procs=0;
   //TOFIX this should be part of the helper class 
#ifndef SHARED_LOWLEVEL
  // Only the shared lowlevel runtime needs to iterate over all points
  // on each processor.
  int num_points = 1;
  int num_procs = 0;
  {
   std::set<Processor> all_procs;
   Realm::Machine::get_machine().get_all_processors(all_procs);
   for(std::set<Processor>::const_iterator it = all_procs.begin();
      it != all_procs.end();
      it++){
    if((*it).kind() == Processor::LOC_PROC)
      num_procs++;
   }
  }
  num_local_procs=num_procs;  
#else
  int num_procs = Machine::get_machine()->get_all_processors().size();
  int num_points = rank->proc_grid_size.x[0] * rank->proc_grid_size.x[1] * rank->proc_grid_size.x[2];
#endif
  printf("Attempting to connect %d processors with %d points per processor\n",
         num_procs, num_points);
  Point<2> all_procs_lo, all_procs_hi;
  all_procs_lo.x[0] = all_procs_lo.x[1] = 0;
  all_procs_hi.x[0] = num_procs - 1;
  all_procs_hi.x[1] = num_points - 1;
  Rect<2> all_processes = Rect<2>(all_procs_lo, all_procs_hi); 

  Rect<1> local_procs(0,num_local_procs);
  ArgumentMap arg_map;

  IndexLauncher connect_mpi_launcher(CONNECT_MPI_TASK_ID,
                                       Domain::from_rect<2>(all_processes),
                                       TaskArgument(0, 0),
                                       arg_map);
  IndexLauncher helloworld_launcher(HELLOWORLD_TASK_ID,
                               Domain::from_rect<1>(local_procs),
                               TaskArgument(0, 0),
                               arg_map);

  IndexLauncher handoff_to_mpi_launcher(HANDOFF_TO_MPI_TASK_ID,
                                     Domain::from_rect<2>(all_processes),
                                     TaskArgument(0, 0),
                                     arg_map);

  //run legion_init() from each thead
  FutureMap fm1 = ctx.runtime()->execute_index_space(ctx.legion_ctx(), connect_mpi_launcher);

  //run some legion task here
  fm1.wait_all_results();

  MPILegionInteropHelper->allocate_legion(ctx);
  MPILegionInteropHelper->legion_init(ctx);

  //*********** testing Array  
  
  const int nParts=2;
  Array.allocate_legion(ctx);
  Array.legion_init( ctx);
  Array.copy_mpi_to_legion(ctx);

  double init_value=14;
  Array.legion_init(init_value, ctx);
  //this has to be done on the legion side
 
  double *acc=Array.get_legion_accessor(WRITE_DISCARD, EXCLUSIVE, ctx);
  for (int i=0; i<nElements; i++)
   acc[i]=i;
 
  //needs to be calld every time after get_legion_accessor function
  Array.return_legion_accessor(ctx);
 
  Array.partition_legion(nParts,ctx);
 
  std::cout << "*** Launching Initialization Tasks..." << std::endl;;
    IndexLauncher launcher(
        SPMD_INIT_TID,
        Array.legion_object.launchDomain(),
        TaskArgument(&nElements, sizeof(nElements)),
        ArgumentMap()
    );
  launcher.add_region_requirement(
        RegionRequirement(
            Array.legion_object.logicalPartition(),
            0,
            WRITE_DISCARD,
            EXCLUSIVE,
            Array.legion_object.logicalRegion
        )
    ).add_field(Array.legion_object.fid);

  auto futureMap = ctx.runtime()->execute_index_space(ctx.legion_ctx(), launcher);
  futureMap.wait_all_results(); 
 
  Array.dump_legion("legion Array", 1, ctx);

  MPILegionInteropHelper->add_array_to_storage(&Array);

  MPILegionInteropHelper->copy_data_from_mpi_to_legion(ctx);

  FutureMap fm2 = ctx.runtime()->execute_index_space(ctx.legion_ctx(), helloworld_launcher);
  fm2.wait_all_results();
  //handoff to MPI
  ctx.runtime()->execute_index_space(ctx.legion_ctx(), handoff_to_mpi_launcher);
 }
}

/* ------------------------------------------------------------------------- */
void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  printf ("helloworld \n");
}

/* ------------------------------------------------------------------------- */
void spmd_init_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
 static const int ridParams = 0;
 const int nElements = *(int *)legiontask->args;
 double *acc = Array.legion_accessor (regions[0],ctx, runtime);
 assert (acc); 

  int counter=0;
  acc[0]=111;

}

/* ------------------------------------------------------------------------- */
void my_init_legion(){
  //should be very first in the main function 
  //TOFIX need to be moved to the flecsi main
  MPILegion_Init();

  HighLevelRuntime::register_legion_task< helloworld_mpi_task >( HELLOWORLD_TASK_ID,
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "hellowrld_task");

 HighLevelRuntime::register_legion_task< spmd_init_task >( SPMD_INIT_TID,
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "spmd_init_task");

  const InputArgs &args = HighLevelRuntime::get_input_args();
  flecsi::execution_t<flecsi::mpilegion_execution_policy_t>::execute_driver(flecsi::mpilegion_top_level_task,1,args.argv);

   MPILegionInteropHelper->legion_configure();

  MPILegionArray<double, nElements> *ArrayDouble= new MPILegionArray<double, nElements>;
  MPILegionArray<int, nElements> *ArrayInt= new MPILegionArray<int, nElements>;
  MPILegionArray<double, nElements> *ArrayResult= new MPILegionArray<double, nElements>;

  MPILegionInteropHelper->add_array_to_storage(ArrayDouble);
  MPILegionInteropHelper->add_array_to_storage(ArrayInt);
  MPILegionInteropHelper->add_array_to_storage(ArrayResult);

  assert (MPILegionInteropHelper->storage_size()==3);

  MPILegionInteropHelper->mpi_init();

  ArrayDouble->mpi_init(1.1);
  ArrayInt->mpi_init(4);
  int *AInt = ArrayInt->mpi_accessor();
  double *ADouble = ArrayDouble->mpi_accessor();
  double *AResult=ArrayResult->mpi_accessor();
  for (int i=0; i< nElements; i++)
  {
    AResult[i]=AInt[i]*ADouble[i];
  }

  assert(AResult[0]==4.4);

  Array.mpi_init();
  int size=Array.size();
  assert (size=nElements);

  MPILegionInteropHelper->handoff_to_legion();

  MPILegionInteropHelper->wait_on_legion(); 

  delete ArrayDouble;
  delete ArrayInt;
  delete ArrayResult;
}

/* ------------------------------------------------------------------------- */

#define execute(task, ...) \
  execution_type::execute_task(task, ##__VA_ARGS__)

/* ------------------------------------------------------------------------- */
TEST(mpi_with_legion, simple) {
   ASSERT_LT(execute(world_size), 1);

   my_init_legion(); 
 
} // TEST

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
