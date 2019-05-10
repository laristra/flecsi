/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include <flecsi/execution/common/command_line_options.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/internal_task.h>
#include <flecsi/execution/legion/mapper.h>
#include <flecsi/execution/legion/tasks.h>
#include <flecsi/utils/const_string.h>

namespace flecsi {
namespace execution {

int
legion_context_policy_t::start(int argc, char ** argv, variables_map & vm) {
  using namespace Legion;

  /*
    Setup Legion top-level task.
   */

  Runtime::set_top_level_task_id(FLECSI_TOP_LEVEL_TASK_ID);

  {
    Legion::TaskVariantRegistrar registrar(
      FLECSI_TOP_LEVEL_TASK_ID, "runtime_driver");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_inner();
    registrar.set_replicable();
    Runtime::preregister_task_variant<top_level_task>(
      registrar, "runtime_driver");
  } // scope

  /*
    Register tasks.
   */

  // clang-format off
  for(auto & t : task_registry_) {
    std::get<4>(t.second)(
      std::get<0>(t.second) /* tid */,
      std::get<1>(t.second) /* processor */,
      std::get<2>(t.second) /* launch */,
      std::get<3>(t.second) /* name */);
  } // for
  // clang-format on

  /*
    Arg 0: MPI has initial control (true).
    Arg 1: Number of MPI participants (1).
    Arg 2: Number of Legion participants (1).
   */

  handshake_ = Legion::Runtime::create_handshake(true, 1, 1);

  /*
    Register custom mapper.
   */

  Runtime::add_registration_callback(mapper_registration);

  /*
    Configure interoperability layer.
   */

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  process_ = rank;
  processes_ = size;

  Legion::Runtime::configure_MPI_interoperability(rank);

  /*
    Register reduction operations.
   */

  for(auto & ro : context_t::instance().reduction_registry()) {
    ro.second();
  } // for

  /*
    Handle command-line arguments.
   */

  std::vector<char *> largv;
  largv.push_back(argv[0]);

  threads_per_process_ = vm[FLECSI_TPP_OPTION_STRING].as<size_t>();

  if(threads_per_process_ > 1) {
    largv.push_back(const_cast<char *>(std::string("-ll:cpu").c_str()));
    largv.push_back(
      const_cast<char *>(std::to_string(threads_per_process_).c_str()));
  } // if

  threads_ = processes_ * threads_per_process_;

  /*
    Start Legion runtime.
   */

  Runtime::start(largv.size(), largv.data(), true);

  {
    flog_tag_guard(context);
    flog(internal) << "Legion runtime started" << std::endl;
  }

  handoff_to_legion();
  wait_on_legion();

  while(mpi_active_) {
    invoke_mpi_task();
    mpi_active_ = false;
    handoff_to_legion();
    wait_on_legion();
  }

  // Make sure that the flusher thread executes at least one cycle.
  __flog_internal_wait_on_flusher();

  Legion::Runtime::wait_for_shutdown();

  return context_t::instance().exit_status();
} // legion_context_policy_t::start

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::unset_call_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::unset_call_mpi(Legion::Context & ctx,
  Legion::Runtime * runtime) {
  {
    flog_tag_guard(context);
    flog(internal) << "In unset_call_mpi" << std::endl;
  }

  const auto tid =
    context_t::instance().task_id<flecsi_internal_hash(unset_call_mpi_task)>();

  Legion::ArgumentMap arg_map;
  // IRINA DEBUG check number of processors
  Legion::IndexLauncher launcher(tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, launcher);
  fm.wait_all_results(true);
} // legion_context_policy_t::unset_call_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::handoff_to_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::handoff_to_mpi(Legion::Context & ctx,
  Legion::Runtime * runtime) {
  const auto tid =
    context_t::instance().task_id<flecsi_internal_hash(handoff_to_mpi_task)>();

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher handoff_to_mpi_launcher(tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  handoff_to_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, handoff_to_mpi_launcher);

  fm.wait_all_results(true);
} // legion_context_policy_t::handoff_to_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::wait_on_mpi.
//----------------------------------------------------------------------------//

Legion::FutureMap
legion_context_policy_t::wait_on_mpi(Legion::Context & ctx,
  Legion::Runtime * runtime) {
  const auto tid =
    context_t::instance().task_id<flecsi_internal_hash(wait_on_mpi_task)>();

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher wait_on_mpi_launcher(tid,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  wait_on_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, wait_on_mpi_launcher);

  fm.wait_all_results(true);

  return fm;
} // legion_context_policy_t::wait_on_mpi

//----------------------------------------------------------------------------//
// Implementation of legion_context_policy_t::connect_with_mpi.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::connect_with_mpi(Legion::Context & ctx,
  Legion::Runtime * runtime) {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  LegionRuntime::Arrays::Rect<1> launch_bounds(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(size - 1));

  context_t::instance().set_all_processes(launch_bounds);

  const std::map<int, Legion::AddressSpace> & forward_mapping =
    runtime->find_forward_MPI_mapping();

#if 0
  for(std::map<int, Legion::AddressSpace>::const_iterator it =
        forward_mapping.begin();
      it != forward_mapping.end(); it++)
    printf(
      "MPI Rank %d maps to Legion Address Space %d\n", it->first, it->second);
#endif
} // legion_context_policy_t::connect_with_mpi

//----------------------------------------------------------------------------//
// Implementation of initialize_global_topology.
//----------------------------------------------------------------------------//

const size_t flecsi_global_topology_hash =
  typeid(global_topology_t).hash_code();
  //flecsi_internal_hash(global_topology_t);

void
legion_context_policy_t::initialize_global_topology() {
  using namespace Legion;

  global_runtime_data_.id = 4096;

  auto legion_runtime_ = Legion::Runtime::get_runtime();
  auto legion_context_ = Legion::Runtime::get_context();

  LegionRuntime::Arrays::Rect<1> bounds(
    LegionRuntime::Arrays::Point<1>(0), LegionRuntime::Arrays::Point<1>(1));

  Domain dom(Domain::from_rect<1>(bounds));

  global_runtime_data_.index_space =
    legion_runtime_->create_index_space(legion_context_, dom);

  global_runtime_data_.field_space =
    legion_runtime_->create_field_space(legion_context_);

  FieldAllocator allocator = legion_runtime_->create_field_allocator(
    legion_context_, global_runtime_data_.field_space);

  auto const & field_store = context_t::instance().get_field_info_store(
    flecsi_global_topology_hash, flecsi::data::storage_label_t::global);

  for(auto const & fi : field_store.data()) {
    allocator.allocate_field(fi.type_size, fi.fid);
  } // for

  global_runtime_data_.logical_region =
    legion_runtime_->create_logical_region(legion_context_,
      global_runtime_data_.index_space,
      global_runtime_data_.field_space);
} // legion_context_policy_t::initialize_global_topology

//----------------------------------------------------------------------------//
// Implementation of initialize_color_topology.
//----------------------------------------------------------------------------//

void
legion_context_policy_t::initialize_color_topology() {
} // legion_context_policy_t::initialize_color_topology

} // namespace execution
} // namespace flecsi
