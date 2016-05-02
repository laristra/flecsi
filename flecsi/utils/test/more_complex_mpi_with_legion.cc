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
#include "legion.h"

// user includes
#include "flecsi/utils/mpi_legion_interoperability/legion_handshake.h"
#include "flecsi/utils/mpi_legion_interoperability/mapper.h"
#include "flecsi/utils/mpi_legion_interoperability/task_ids.h"
#include "flecsi/execution/mpi_execution_policy.h"
#include "flecsi/execution/task.h"

using execution_t = flecsi::execution_t<flecsi::mpi_execution_policy_t>;
using return_type_t = execution_t::return_type_t;

enum TaskIDs{
 TOP_LEVEL_TASK_ID         =0x00000100,
 PRINT_TASK_ID             =0x00000300,
 COMPUTE_SOMETHING_ID      =0x00000400,
 INIT_FIELD_TASK_ID        =0x00000500,
};

enum FieldIDs {
  CELLS_ON_CELL,
  EDGES_ON_CELL,
  SOMETHING,
};

ExtLegionHandshake *handshake;

return_type_t world_size() {
  int world_size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  return 0;
}

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{

  int nCells=100;
  int nEdges=200;
  int maxEdges=8; 
  int elem_rect_hi_val=nCells*maxEdges - 1;

  int num_subregions = 4;
  int color_hi_val= sqrt(num_subregions) - 1;
  int patch_val= nCells*maxEdges/(num_subregions*num_subregions);

  int num_procs=0;
  int num_local_procs=0;

//TOFIX: change to the correct allocation
//what "all_procs" index space represents?
  std::set<Processor> all_procs;
  Realm::Machine::get_machine().get_all_processors(all_procs);
  int num_loc_procs = 0;
  for(std::set<Processor>::const_iterator it = all_procs.begin();
      it != all_procs.end();
      it++){
    if((*it).kind() == Processor::LOC_PROC)
      num_loc_procs++;
    num_procs++;
   }

  Rect<2> all_processes(make_point(0,0),make_point(num_loc_procs-1,num_loc_procs-1));
  Rect<1> local_procs(0,num_local_procs);
  ArgumentMap arg_map;
  //connect to MPI
  IndexLauncher connect_mpi_launcher(CONNECT_MPI_TASK_ID,
                                       Domain::from_rect<2>(all_processes),
                                       TaskArgument(0, 0),
                                       arg_map);

  FutureMap fm1 = runtime->execute_index_space(ctx, connect_mpi_launcher);


  Domain color_domain;
  Point<2> color_lo; color_lo.x[0] = 0; color_lo.x[1] = 0;
  Point<2> color_hi; color_hi.x[0] = color_hi.x[1] = num_subregions;
  Rect<2> color_bounds(color_lo, color_hi); 
  color_domain = Domain::from_rect<2>(color_bounds);
  

  FieldSpace fs = runtime->create_field_space(ctx);
  {
    FieldAllocator allocator = 
      runtime->create_field_allocator(ctx, fs);
    allocator.allocate_field(sizeof(int),CELLS_ON_CELL);
    allocator.allocate_field(sizeof(int),EDGES_ON_CELL);
  }

  FieldSpace fs_something = runtime->create_field_space(ctx);
  {
    FieldAllocator allocator =
      runtime->create_field_allocator(ctx, fs_something);
    allocator.allocate_field(sizeof(int),SOMETHING);
  }

  Domain elem_domain;
  Point<2> elem_rect_lo; elem_rect_lo.x[0] = 0; elem_rect_lo.x[1]=0;
  Point<2> elem_rect_hi; elem_rect_hi.x[0] = nCells; elem_rect_hi.x[1] = maxEdges;
  Rect<2> elem_rect( elem_rect_lo, elem_rect_hi );
  elem_domain = Domain::from_rect<2>(elem_rect);
 
  IndexSpace is = runtime->create_index_space(ctx, elem_domain);

  LogicalRegion elem_lr = runtime->create_logical_region(ctx, is, fs);
  LogicalRegion something_lr = runtime->create_logical_region(ctx, is, fs_something);

  IndexPartition ip;
  Point<2> patch_color; patch_color.x[0] = nCells/num_subregions;  patch_color.x[1] = maxEdges/num_subregions;
  Blockify<2> coloring(patch_color); 
  ip  = runtime->create_index_partition(ctx, is, coloring);
  runtime->attach_name(ip, "ip");

  LogicalPartition elem_lp = runtime->get_logical_partition(ctx, elem_lr,ip);
  runtime->attach_name(elem_lp, "elem_lp");

  LogicalPartition something_lp = runtime->get_logical_partition(ctx, something_lr,ip);
  runtime->attach_name(something_lp, "something_lp");


  IndexLauncher init_launcher(INIT_FIELD_TASK_ID, color_domain,
      TaskArgument(&patch_val, sizeof(patch_val)), ArgumentMap());

  init_launcher.add_region_requirement(
      RegionRequirement(elem_lp, 0,
        WRITE_DISCARD, EXCLUSIVE, elem_lr));
  init_launcher.add_field(0, CELLS_ON_CELL);
  FutureMap fm2 = runtime->execute_index_space(ctx, init_launcher);  

  init_launcher.region_requirements[0].privilege_fields.clear();
  init_launcher.region_requirements[0].instance_fields.clear();
  init_launcher.region_requirements[0].add_field(EDGES_ON_CELL);
  fm2 = runtime->execute_index_space(ctx, init_launcher);

  IndexLauncher something_launcher(COMPUTE_SOMETHING_ID, color_domain,
      TaskArgument(&patch_val, sizeof(patch_val)), ArgumentMap());
  something_launcher.add_region_requirement(
      RegionRequirement(elem_lp, 0,
        READ_ONLY, EXCLUSIVE, elem_lr));
  something_launcher.region_requirements[0].add_field(CELLS_ON_CELL);
  something_launcher.region_requirements[0].add_field(EDGES_ON_CELL);
  something_launcher.add_region_requirement(
      RegionRequirement(something_lp, 0,
        WRITE_DISCARD, EXCLUSIVE, something_lr));
  something_launcher.region_requirements[1].add_field(SOMETHING);
  FutureMap fm3 = runtime->execute_index_space(ctx, something_launcher);

  //print the results out
  std::vector<Future> future_tmp;
   TaskLauncher print_launcher(PRINT_TASK_ID, 
      TaskArgument(0, 0));
  print_launcher.add_region_requirement(
      RegionRequirement(something_lr, READ_ONLY, EXCLUSIVE, something_lr));
  print_launcher.add_field(0, SOMETHING);
   future_tmp.push_back(runtime->execute_task(ctx, print_launcher));

}

void init_field_task(const Task *legiontask,
                     const std::vector<PhysicalRegion> &regions,
                     Context ctx, HighLevelRuntime *runtime)
{

  printf ("inside init_task \n");

  assert(regions.size() == 1);
  assert(legiontask->regions.size() == 1);
  assert(legiontask->regions[0].privilege_fields.size() == 1);
  int extent = *(const int*) legiontask->args;


  FieldID fid = *(legiontask->regions[0].privilege_fields.begin());
  const int point = legiontask->index_point.point_data[0];
  printf("Initializing field %d for block %d...\n", fid, point);
  RegionAccessor<AccessorType::Generic, int> acc =
    regions[0].get_field_accessor(fid).typeify<int>();
  // Note here that we get the domain for the subregion for
  // this task from the runtime which makes it safe for running
  // both as a single task and as part of an index space of tasks.
  Domain dom = runtime->get_index_space_domain(ctx,
      legiontask->regions[0].region.get_index_space());

  Rect<2> rect = dom.get_rect<2>();
  int counter=0;
  for (GenericPointInRectIterator<2> pir(rect); pir; pir++) {
      counter++;
      acc.write(DomainPoint::from_point<2>(pir.p),counter);
   }

}

void compute_something_task(const Task *legiontask,
                     const std::vector<PhysicalRegion> &regions,
                     Context ctx, HighLevelRuntime *runtime)
{
 printf ("inside compute_task \n");
  assert(regions.size() == 2);
  assert(legiontask->regions.size() == 2);
  assert(legiontask->arglen == sizeof(int));
  int extent = *(const int*) legiontask->args; 

  RegionAccessor<AccessorType::Generic, int> acc_cells =
    regions[0].get_field_accessor(CELLS_ON_CELL).typeify<int>();
  RegionAccessor<AccessorType::Generic, int> acc_edges =
    regions[0].get_field_accessor(EDGES_ON_CELL).typeify<int>();
  RegionAccessor<AccessorType::Generic, int> acc_something =
    regions[1].get_field_accessor(SOMETHING).typeify<int>();

  Domain dom = runtime->get_index_space_domain(ctx,
      legiontask->regions[0].region.get_index_space());

  Rect<2> rect = dom.get_rect<2>();
  for (GenericPointInRectIterator<2> pir(rect); pir; pir++) {
    int temp = acc_cells.read (DomainPoint::from_point<2>(pir.p))
               +acc_edges.read (DomainPoint::from_point<2>(pir.p));
    acc_something.write (DomainPoint::from_point<2>(pir.p),temp);
  }
}

int print_task(const Task *legiontask,
                const std::vector<PhysicalRegion> &regions,
                Context ctx, HighLevelRuntime *runtime)
{
  assert(regions.size() == 1);
  assert(legiontask->regions.size() == 1);
  assert(legiontask->regions[0].privilege_fields.size() == 1);

printf("inside print task\n");
  FieldID src_fid = *(legiontask->regions[0].privilege_fields.begin());

  RegionAccessor<AccessorType::Generic, int> src_acc =
    regions[0].get_field_accessor(src_fid).typeify<int>();

  Domain dom = runtime->get_index_space_domain(ctx,
      legiontask->regions[0].region.get_index_space());
  Rect<2> rect = dom.get_rect<2>();
 
  for (GenericPointInRectIterator<2> pir(rect); pir; pir++){
     printf ( "     %d ", src_acc.read(DomainPoint::from_point<2>(pir.p))); 
  }

  handshake->legion_handoff_to_ext();
  return 0;
}


void connect_mpi_task (const Task *task,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
    handshake->legion_init();
}


void helloworld_mpi_task (const Task *task,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime)
{
  printf ("helloworld \n");
}

void complete_legion_configure(void)
{
   handshake->ext_init();
}

void run_legion_task(void)
{
   handshake->ext_handoff_to_legion();
}


void my_init_legion(){

  handshake = new ExtLegionHandshake(ExtLegionHandshake::IN_EXT, 1, 1);

  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);

  HighLevelRuntime::register_legion_task<top_level_task>( TOP_LEVEL_TASK_ID,
                          Processor::LOC_PROC, true/*single*/, false/*index*/, 
                          AUTO_GENERATE_ID, TaskConfigOptions(), "top_level_task");
  HighLevelRuntime::register_legion_task< connect_mpi_task>( CONNECT_MPI_TASK_ID, 
                          Processor::LOC_PROC, false/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "connect_mpi_task");
  HighLevelRuntime::register_legion_task<init_field_task>(INIT_FIELD_TASK_ID,
                          Processor::LOC_PROC, true/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true), "init_field");  
  HighLevelRuntime::register_legion_task<compute_something_task>(COMPUTE_SOMETHING_ID,
                          Processor::LOC_PROC, true/*single*/, true/*index*/,
                          AUTO_GENERATE_ID, TaskConfigOptions(true), "init_field");
  HighLevelRuntime::register_legion_task<int,print_task>( PRINT_TASK_ID,
                          Processor::LOC_PROC, true/*single*/, false/*index*/, 0,
                           TaskConfigOptions(), "print_task");

  const InputArgs &args = HighLevelRuntime::get_input_args();

  HighLevelRuntime::set_registration_callback(mapper_registration);

  HighLevelRuntime::start(args.argc, args.argv, true);

  complete_legion_configure();

  run_legion_task();  

  handshake->ext_wait_on_legion(); 

}

#define execute(task, ...) \
  execution_t::execute_task(task, ##__VA_ARGS__)

TEST(more_complex_mpi_with_legion, simple) {
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
