/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_interop_init_h
#define flecsi_execution_test_interop_init_h

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>
#include <type_traits> // std::is_same

// user includes
//#include "flecsi/execution/mpilegion/legion_handshake.h"
//#include "flecsi/execution/mpilegion/mapper.h"
//#include "flecsi/execution/task_ids.h"
//#include "flecsi/execution/task.h"

#include "flecsi/execution/execution.h"
#include "flecsi/utils/any.h"
#include "flecsi/partition/index_partition.h"


using namespace flecsi::execution;
using index_partition_t = flecsi::dmp::index_partition__<size_t>;

/// Task IDs
enum TaskIDs{
 TOP_LEVEL_TASK_ID         =0x00000010,
 HELLOWORLD_TASK_ID        =0x00000100,
 SPMD_INIT_TID             =0x00000200,
};




using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;


void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime);

void helloworld_mpi_task (const Task *legiontask,
                      const std::vector<PhysicalRegion> &regions,
                      Context ctx, HighLevelRuntime *runtime);



class InteropS {
public: 
  static InteropS& getInstance() {
    static InteropS instance;
    return instance;
  }
private:
  InteropS() {}
  //InteropS(InteropS const&);
  //void operator=(InteropS const&);
  
public:
  InteropS(InteropS const &) = delete;
  void operator=(InteropS const&) = delete;
  mpi_legion_interop_t helper; 
};
       
   
//static mpi_legion_interop_t InteropHelper;


///
/// \file
/// \date Initial file creation: Dec 21, 2016
///

void
inline
test_init(
  int argc,
  char ** argv
)
{

  
  int rank = -1, size = -1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  printf("Hello from MPI process %d of %d\n", rank, size);

  InteropS::getInstance().helper.legion_configure();

  
// Configure the Legion runtime with the rank of this process
  Legion::Runtime::configure_MPI_interoperability(rank);
  // Register our task variants
  {
    TaskVariantRegistrar top_level_registrar(TOP_LEVEL_TASK_ID);
    top_level_registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<top_level_task>(top_level_registrar,
                                                      "Top Level Task");
    Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  }
  {
    TaskVariantRegistrar helloworld_registrar(HELLOWORLD_TASK_ID);
    helloworld_registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<helloworld_mpi_task>(helloworld_registrar,
                                                        "MPI Interop Task");
  }


  InteropS::getInstance().helper.register_tasks();
  InteropS::getInstance().helper.initialize();

  // This will initialize the legion runtime, so you need to
  // do anything else before you get here.
  HighLevelRuntime::start(argc, argv, true/*background*/);
  
  
  InteropS::getInstance().helper.handoff_to_legion();

  InteropS::getInstance().helper.wait_on_legion();

  //check data_storage_
  index_partition_t ip;
  double A=3.14;
  InteropS::getInstance().helper.data_storage_.push_back(flecsi::utils::any_t(ip));
  InteropS::getInstance().helper.data_storage_.push_back(flecsi::utils::any_t(A));

  InteropS::getInstance().helper.handoff_to_legion();
 
  InteropS::getInstance().helper.wait_on_legion();
  Legion::Runtime::wait_for_shutdown();
  std::cout<<"back to MPI to finalize"<<std::endl;


} // test_init

#endif // flecsi_execution_task_init_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
