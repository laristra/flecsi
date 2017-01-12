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

#ifndef flecsi_execution_mpilegion_mpi_legion_interop_h
#define flecsi_execution_mpilegion_mpi_legion_interop_h

#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <string>

#include <legion.h>
#include <mpi.h>
#include <realm.h>

#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mapper.h"
#include "flecsi/execution/task_ids.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/utils/any.h"

///
/// \file 
/// \date Initial file creation: Jul 2016
///

namespace flecsi{
namespace execution{

//----------------------------------------------------------------------------//
/// A legion task that calls legion_handoff_to_ext function from
/// the handshake class that switches form Legion to MPI runtime. 
/// This task is an index task that will be executed on as 
/// many processes as MPI has been called from.
///
//----------------------------------------------------------------------------//

inline
void
handoff_to_mpi_task(
  const Legion::Task * legiontask,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime)
{
std::cout <<"inside handoff_to_mpi task" <<std::endl;
    ext_legion_handshake_t::instance().legion_handoff_to_mpi();
} // handoff_to_mpi_task

//----------------------------------------------------------------------------//
///
/// A legion task that waits for every MPI thread to finish
///
//----------------------------------------------------------------------------//

inline
void
wait_on_mpi_task(
  const Legion::Task * legiontask,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime
)
{
  ext_legion_handshake_t::instance().legion_wait_on_mpi();
} // wait_on_mpi_task

//----------------------------------------------------------------------------//
///
/// In purpose to tell MPI runtime that there is an MPI task that has to be 
/// executed, we have to switch call_mpi boolian variable on every MPI thread.
/// his task is an index task that will be executed on as many processes as MPI 
/// has been called from.
///
//----------------------------------------------------------------------------//

inline
void
unset_call_mpi_task(
  const Legion::Task *legiontask,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime
)
{
  ext_legion_handshake_t::instance().call_mpi_=false;
} // unset_call_mpi_task


//----------------------------------------------------------------------------//
///
/// \class mpi_legion_interop
/// mpi_legion_interop class is used to wrap legion-mpi handshaking routines 
/// in a FLeCSI-friendly interface
///
//----------------------------------------------------------------------------//

struct mpi_legion_interop_t
{
  ///
  /// Constructor.
  ///
  mpi_legion_interop_t() {};

  ///
  /// Copy constructor.
  ///
  mpi_legion_interop_t(const mpi_legion_interop_t &) = delete;

  ///
  /// Assign operator.
  ///
  mpi_legion_interop_t& operator=(const mpi_legion_interop_t &) = delete;

  /// 
  /// Destructor.
  ///
  ~mpi_legion_interop_t() {};

  ///
  /// Initialze() method needs to be called before we start Legion runtime. 
  /// The method load a customized mapper that is tuned for MPI-Legion 
  /// interoperability.
  ///
  void
  initialize()
  {
    ext_legion_handshake_t::instance().initialize();
    LegionRuntime::HighLevel::HighLevelRuntime::set_registration_callback(
      mapper_registration);
  } // initialize
 
  ///
  /// This method is used to communicate to  MPI runtime that there is an
  /// MPI task that has to be executed after we switches from Legion to 
  /// MPI. It launches legion Index task that switches call_mpi variable
  /// to true for every MPI process
  ///
  void
  unset_call_mpi(
   LegionRuntime::HighLevel::Context ctx,
   LegionRuntime::HighLevel::HighLevelRuntime *runtime
  )
  {
  	LegionRuntime::HighLevel::ArgumentMap arg_map;

  	LegionRuntime::HighLevel::IndexLauncher unset_call_mpi_launcher(
    	task_ids_t::instance().unset_call_mpi_id,
    	LegionRuntime::HighLevel::Domain::from_rect<1>(all_processes_),
    	LegionRuntime::HighLevel::TaskArgument(0, 0),
    	arg_map);

  	unset_call_mpi_launcher.tag =  MAPPER_FORCE_RANK_MATCH;

  	LegionRuntime::HighLevel::FutureMap fm5 =
    runtime->execute_index_space(ctx,unset_call_mpi_launcher);

  	fm5.wait_all_results();
	}//unset_call_mpi
 
  ///
  /// This method creates pthreads mutex on the MPI side and, in case handshake 
  /// is originally created in MPI, waits on when handshake is created on
  /// the Legion side.
  ///
  void
  legion_configure()
  {
    int rank = -1, size = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    Legion::Runtime::configure_MPI_interoperability(rank);
  } // legion_configure

  ///
  /// This method swithches mutex to Legion runtime.
  ///
  void 
  handoff_to_legion()
  {
    ext_legion_handshake_t::instance().mpi_handoff_to_legion();
  } // handoff_to_legion

  ///
  /// This method waits untill mutex switched to MPI runtime and run MPI.
  ///
  void 
  wait_on_legion()
  {
    ext_legion_handshake_t::instance().mpi_wait_on_legion();
  } // wait_on_legion

  ///
  /// This method registers all Legion tasks used in the mpi_legion_interop_t 
  ///class.
  ///
  static
	void 
	register_tasks()
	{
  	LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
    	<handoff_to_mpi_task>(
	    task_ids_t::instance().handoff_to_mpi_task_id,
    	LegionRuntime::HighLevel::Processor::LOC_PROC,
    	false/*single*/, true/*index*/,
    	AUTO_GENERATE_ID,
    	LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
    	"handoff_to_mpi_task");

  	LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
    	<wait_on_mpi_task>(
    	task_ids_t::instance().wait_on_mpi_task_id,
    	LegionRuntime::HighLevel::Processor::LOC_PROC,
    	false/*single*/, true/*index*/,
    	AUTO_GENERATE_ID,
    	LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
    	"wait_on_mpi_task");

   	LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
     	<unset_call_mpi_task>(
     	task_ids_t::instance().unset_call_mpi_id,
     	LegionRuntime::HighLevel::Processor::LOC_PROC,
     	false, true, AUTO_GENERATE_ID,
     	LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
     	"unset_call_mpi_task");

	}//register_tasks

  ///
  /// Helper function that calculates number of availabe processes (the number
  /// of processes that MPI has been called from).
  ///
  void 
  calculate_number_of_procs ()
  {
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
  	this->all_processes_ =  LegionRuntime::Arrays::Rect<1>(0, num_ranks-1);
	}//calculate_number_of_procs

  ///
  /// This method calls a legion's set-up handhsking between mpi and legion
  /// on the legion side
  ///
  void 
  connect_with_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime
  )
	{
		calculate_number_of_procs();

	// Both the application and Legion mappers have access to
  // the mappings between MPI Ranks and Legion address spaces
  // The reverse mapping goes the other way
  const std::map<int,Legion::AddressSpace> &forward_mapping =
    runtime->find_forward_MPI_mapping();
  for (std::map<int,Legion::AddressSpace>::const_iterator it =
        forward_mapping.begin(); it != forward_mapping.end(); it++)
      printf("MPI Rank %d maps to Legion Address Space %d\n",
            it->first, it->second);
  } // connect_with_mpi/  


  ///
  /// This method calls an Index task that switches form Legion to MPI runtime.
  ///
  void 
  handoff_to_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime
  )
  {
	 	LegionRuntime::HighLevel::ArgumentMap arg_map;

  	LegionRuntime::HighLevel::IndexLauncher handoff_to_mpi_launcher(
    	task_ids_t::instance().handoff_to_mpi_task_id,
    	LegionRuntime::HighLevel::Domain::from_rect<1>(all_processes_),
    	LegionRuntime::HighLevel::TaskArgument(0, 0),
  	  arg_map);

 	 LegionRuntime::HighLevel::FutureMap fm2 =
    	runtime->execute_index_space(ctx, handoff_to_mpi_launcher);

  	fm2.wait_all_results();
	} // handoff_to_mpi
 
  ///
  /// This method waits on all MPI processes to finish all tasks and
  /// switch to Legion runtime
  ///
  LegionRuntime::HighLevel::FutureMap 
  wait_on_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime
  )
	{
   	LegionRuntime::HighLevel::ArgumentMap arg_map;
  	LegionRuntime::HighLevel::IndexLauncher wait_on_mpi_launcher(
    	task_ids_t::instance().wait_on_mpi_task_id,
    	LegionRuntime::HighLevel::Domain::from_rect<1>(all_processes_),
    	LegionRuntime::HighLevel::TaskArgument(0, 0),
    	arg_map);

  	LegionRuntime::HighLevel::FutureMap fm3 =
    	runtime->execute_index_space(ctx, wait_on_mpi_launcher);

  	fm3.wait_all_results();

  	return fm3;
	}//wait_on_mpi
 
  LegionRuntime::Arrays::Rect<1> all_processes_;

  std::vector<flecsi::utils::any_t> data_storage_;

}; // mpi_legion_interop_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_mpilegion_mpi_legion_interop_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
