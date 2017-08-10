/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include "flecsi/execution/legion/context_policy.h"

#include <iostream>

#include "flecsi/execution/legion/legion_tasks.h"
#include "flecsi/execution/legion/mapper.h"
#include "flecsi/data/storage.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// External declaration of context state. Please read the description and
// limitations of this state in the header file.
//----------------------------------------------------------------------------//

thread_local std::unordered_map<size_t,
  std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;  

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::initialize.
//----------------------------------------------------------------------------//

int
legion_context_policy_t::initialize(
  int argc,
  char ** argv
)
{
  using namespace Legion;

  // Register top-level task
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
 
  {
    Legion::TaskVariantRegistrar registrar(TOP_LEVEL_TASK_ID, "runtime_driver");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<runtime_driver>(registrar,
      "runtime_driver");
  }

  // Register tasks
  for(auto & t: task_registry_) {
    std::get<4>(t.second)(
      std::get<0>(t.second) /* tid */,
      std::get<1>(t.second) /* processor */,
      std::get<2>(t.second) /* launch */,
      std::get<3>(t.second) /* name */);
  } // for

  // Intialize MPI/Legion interoperability layer.
  handshake_ = Legion::Runtime::create_handshake(
    true, // MPI has initial control
    1,    // MPI partiticipants
    1     // Legion participants
  );

  // Register our mapper
  Runtime::add_registration_callback(mapper_registration);

  // Configure interoperability layer.
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  color_ = rank;
  colors_ = size;

  Legion::Runtime::configure_MPI_interoperability(rank);

  Runtime::register_reduction_op<MaxReductionOp>(MaxReductionOp::redop_id);
  Runtime::register_reduction_op<MinReductionOp>(MinReductionOp::redop_id);

  // Start the Legion runtime
  Runtime::start(argc, argv, true);

  handoff_to_legion();
  wait_on_legion();

  while(mpi_active_) {
    invoke_mpi_task();
    handoff_to_legion();
    wait_on_legion();
  }
  
  int version, subversion;
  MPI_Get_version(&version, &subversion);
  if(version==3 && subversion>0) {
    Legion::Runtime::wait_for_shutdown();
  } // if

  return 0;
} // legion_context_policy_t::initialize

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::unset_call_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::unset_call_mpi(
  Legion::Context & ctx,
  Legion::HighLevelRuntime * runtime
)
{
  {
  clog_tag_guard(context);
  clog(info) << "In unset_call_mpi" << std::endl;
  }

  const auto tid = context_t::instance().task_id<
    __flecsi_internal_task_key(unset_call_mpi_task)>();

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher launcher(
    tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );

  launcher.tag = MAPPER_FORCE_RANK_MATCH;

  auto fm = runtime->execute_index_space(ctx, launcher);

  fm.wait_all_results();
} // legion_context_policy_t::unset_call_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::handoff_to_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::handoff_to_mpi(
  Legion::Context & ctx,
  Legion::HighLevelRuntime * runtime
)
{
  const auto tid = context_t::instance().task_id<
    __flecsi_internal_task_key(handoff_to_mpi_task)>();

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher handoff_to_mpi_launcher(
    tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );

  auto fm = runtime->execute_index_space(ctx, handoff_to_mpi_launcher);

  fm.wait_all_results();
} // legion_context_policy_t::handoff_to_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::wait_on_mpi.
//----------------------------------------------------------------------------//

Legion::FutureMap
legion_context_policy_t::wait_on_mpi(
  Legion::Context & ctx,
  Legion::HighLevelRuntime * runtime
)
{
  const auto tid = context_t::instance().task_id<
    __flecsi_internal_task_key(wait_on_mpi_task)>();

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher wait_on_mpi_launcher(
    tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );

  auto fm = runtime->execute_index_space(ctx, wait_on_mpi_launcher);

  fm.wait_all_results();

  return fm;    
} // legion_context_policy_t::wait_on_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::connect_with_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::connect_with_mpi(
  Legion::Context & ctx,
  Legion::HighLevelRuntime * runtime
)
{
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  context_t::instance().set_all_processes(
    LegionRuntime::Arrays::Rect<1>(0, size-1));

  // FIXME: Does this do anything?
  // Both the application and Legion mappers have access to
  // the mappings between MPI Ranks and Legion address spaces
  // The reverse mapping goes the other way
  const std::map<int, Legion::AddressSpace> & forward_mapping =
    runtime->find_forward_MPI_mapping();
} // legion_context_policy_t::connect_with_mpi

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
