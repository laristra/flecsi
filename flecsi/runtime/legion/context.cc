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

#include <flecsi/data.hh>
#include <flecsi/data/legion/data_policy.hh>
#include <flecsi/execution/command_line_options.hh>
#include <flecsi/execution/launch.hh>
#include <flecsi/execution/legion/task_wrapper.hh>
#include <flecsi/runtime/legion/context.hh>
#include <flecsi/runtime/legion/mapper.hh>
#include <flecsi/runtime/legion/tasks.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/const_string.hh>

namespace flecsi::runtime {

using namespace boost::program_options;
using execution::legion::task_id;

int
context_t::start(int argc, char ** argv, variables_map & vm) {
  using namespace Legion;

  /*
    Setup Legion top-level task.
   */

  Runtime::set_top_level_task_id(FLECSI_TOP_LEVEL_TASK_ID);

  {
    Legion::TaskVariantRegistrar registrar(
      FLECSI_TOP_LEVEL_TASK_ID, "runtime_driver");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    //    registrar.set_inner();
    registrar.set_replicable();
    Runtime::preregister_task_variant<top_level_task>(
      registrar, "runtime_driver");
  } // scope

  /*
    Setup internal launch domains.
   */

  set_launch_domain_size(single, 1);
  set_launch_domain_size(index, 0);

  /*
    Register tasks.
   */

  {
    flog_tag_guard(context);
    flog_devel(info) << "Invoking task registration callbacks" << std::endl;
  }

  for(auto && p : task_registry_)
    p();

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

  std::string tpp; // lives past start() (which is hopefully enough)
  std::vector<char *> largv;
  largv.push_back(argv[0]);

  threads_per_process_ = vm[FLECSI_TPP_OPTION_STRING].as<size_t>();

  if(threads_per_process_ > 1) {
    largv.push_back(const_cast<char *>("-ll:cpu"));
    tpp = std::to_string(threads_per_process_);
    largv.push_back(tpp.data());
  } // if

  threads_ = processes_ * threads_per_process_;

  /*
    Start Legion runtime.
   */

  {
    flog_tag_guard(context);
    flog_devel(info) << "Starting Legion runtime" << std::endl;
  }

  Runtime::start(largv.size(), largv.data(), true);

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
} // context_t::start

//----------------------------------------------------------------------------//
// Implementation of context_t::unset_call_mpi.
//----------------------------------------------------------------------------//

void
context_t::unset_call_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
  {
    flog_tag_guard(context);
    flog_devel(info) << "In unset_call_mpi" << std::endl;
  }

  Legion::ArgumentMap arg_map;
  // IRINA DEBUG check number of processors
  Legion::IndexLauncher launcher(task_id<unset_call_mpi_task>,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, launcher);
  fm.wait_all_results(true);
} // context_t::unset_call_mpi

//----------------------------------------------------------------------------//
// Implementation of context_t::handoff_to_mpi.
//----------------------------------------------------------------------------//

void
context_t::handoff_to_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher handoff_to_mpi_launcher(task_id<handoff_to_mpi_task>,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  handoff_to_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, handoff_to_mpi_launcher);

  fm.wait_all_results(true);
} // context_t::handoff_to_mpi

//----------------------------------------------------------------------------//
// Implementation of context_t::wait_on_mpi.
//----------------------------------------------------------------------------//

Legion::FutureMap
context_t::wait_on_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher wait_on_mpi_launcher(task_id<wait_on_mpi_task>,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(NULL, 0),
    arg_map);

  wait_on_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(ctx, wait_on_mpi_launcher);

  fm.wait_all_results(true);

  return fm;
} // context_t::wait_on_mpi

//----------------------------------------------------------------------------//
// Implementation of context_t::connect_with_mpi.
//----------------------------------------------------------------------------//

void
context_t::connect_with_mpi(Legion::Context & ctx, Legion::Runtime * runtime) {
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
} // context_t::connect_with_mpi

//----------------------------------------------------------------------------//
// Implementation of initialize_global_topology.
//----------------------------------------------------------------------------//

void
context_t::initialize_global_topology() {
  using namespace Legion;

  global_topology_instance_.index_space_id = unique_isid_t::instance().next();

  auto legion_runtime_ = Legion::Runtime::get_runtime();
  auto legion_context_ = Legion::Runtime::get_context();

  LegionRuntime::Arrays::Rect<1> bounds(
    LegionRuntime::Arrays::Point<1>(0), LegionRuntime::Arrays::Point<1>(1));

  Domain dom(Domain::from_rect<1>(bounds));

  global_topology_instance_.index_space =
    legion_runtime_->create_index_space(legion_context_, dom);

  global_topology_instance_.field_space =
    legion_runtime_->create_field_space(legion_context_);

  FieldAllocator allocator = legion_runtime_->create_field_allocator(
    legion_context_, global_topology_instance_.field_space);

  /*
    Note: This call to get_field_info_store uses the non-const version
    so that this call works if no fields have been registered. In other parts
    of the code that occur after initialization, the const version of this call
    should be used.
   */

  auto & field_info_store = context_t::instance().get_field_info_store(
    topology::id<topology::global_topology_t>(),
    flecsi::data::storage_label_t::dense);

  for(auto const & fi : field_info_store.field_info()) {
    allocator.allocate_field(fi.type_size, fi.fid);
  } // for

  global_topology_instance_.logical_region =
    legion_runtime_->create_logical_region(legion_context_,
      global_topology_instance_.index_space,
      global_topology_instance_.field_space);
} // context_t::initialize_global_topology

//----------------------------------------------------------------------------//
// Implementation of finalize_global_topology.
//----------------------------------------------------------------------------//

void
context_t::finalize_global_topology() {
  using namespace Legion;

  global_topology_instance_.index_space_id = unique_isid_t::instance().next();

  auto legion_runtime_ = Legion::Runtime::get_runtime();
  auto legion_context_ = Legion::Runtime::get_context();

  legion_runtime_->destroy_logical_region(
    legion_context_, global_topology_instance_.logical_region);

  legion_runtime_->destroy_field_space(
    legion_context_, global_topology_instance_.field_space);

  legion_runtime_->destroy_index_space(
    legion_context_, global_topology_instance_.index_space);
} // context_t::finalize_global_topology

//----------------------------------------------------------------------------//
// Index coloring and topology method implementations.
//----------------------------------------------------------------------------//

void
context_t::initialize_default_index_coloring() {
  index_colorings_.emplace(flecsi_index_coloring.identifier(), processes_);
} // context_t::initialize_default_index_coloring

void
context_t::initialize_default_index_topology() {

  {
    flog_tag_guard(context);
    flog_devel(info) << "Initializing default index topology" << std::endl
                     << "\tidentifier: " << flecsi_index_topology.identifier()
                     << std::endl;
  }

  data::legion_data_policy_t::allocate<topology::index_topology_t>(
    flecsi_index_topology, flecsi_index_coloring);
} // context_t::initialize_default_index_topology

void
context_t::finalize_default_index_topology() {

  {
    flog_tag_guard(context);
    flog_devel(info) << "Finalizing default index topology" << std::endl
                     << "\tidentifier: " << flecsi_index_topology.identifier()
                     << std::endl;
  }

  data::legion_data_policy_t::deallocate<topology::index_topology_t>(
    flecsi_index_topology);
} // context_t::finalize_default_index_topology

} // namespace flecsi::runtime
