/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

/*!
 * \file mpilegion/runtime_driver.cc
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

#include "flecsi/execution/mpilegion/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/logging.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/task_ids.h"
#include "flecsi/data/legion/dense.h"
#include "flecsi/data/data.h"
#include "flecsi/execution/test/mpilegion/sprint_common.h"

#include <legion_utilities.h>

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

#ifndef FLECSI_SPECIALIZATION_DRIVER
  #include "flecsi/execution/default_specialization_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_SPECIALIZATION_DRIVER)
#endif

#include <vector>

namespace flecsi {
namespace execution {

  namespace{

    struct spmd_task_args{
      size_t num_handles;
      size_t num_colors;
    };

    using handle_t = data_handle_t<void, 0, 0, 0>;

  } // namespace

void
mpilegion_runtime_driver(
  const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context ctx,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime
)
{
    std::cout << "mpilegion_runtime_driver started" << std::endl;
                
    context_t & context_ = context_t::instance();
    context_.push_state(utils::const_string_t{"specialization_driver"}.hash(),
      ctx, runtime, task, regions);

    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

    // connect legion with MPI
    context_.interop_helper_.connect_with_mpi(ctx, runtime);
    context_.interop_helper_.wait_on_mpi(ctx, runtime);

    data_client dc;
    int argc = args.argc + 1;
    char **argv;
    argv = (char**)std::malloc(sizeof(char*)*argc);
    std::memcpy(argv, args.argv, args.argc*sizeof(char*));
    argv[argc - 1] = (char*)&dc;

    // run default or user-defined specialization driver 
    specialization_driver(argc, argv);

    //execute SPMD launch that executes user-defined driver

    MustEpochLauncher must_epoch_launcher; 
    LegionRuntime::HighLevel::ArgumentMap arg_map;

    field_ids_t & fid_t =field_ids_t::instance();

    std::vector<handle_t> handles;
    std::vector<size_t> hashes;
    std::vector<size_t> namespaces;
    std::vector<size_t> versions;
    flecsi_get_all_handles(dc, dense, handles, hashes, namespaces, versions);
    
    Serializer task_args_serializer;
    task_args_serializer.serialize(&handles[0], handles.size() * sizeof(handle_t));
    task_args_serializer.serialize(&hashes[0], hashes.size() * sizeof(size_t));
    task_args_serializer.serialize(&namespaces[0], namespaces.size() * sizeof(size_t));
    task_args_serializer.serialize(&versions[0], versions.size() * sizeof(size_t));

    //create phase barriers per handle for SPMD launch from partitions created and
    //registered to the data Client at the specialization_driver
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    // FIXME Will not scale. Coloring in serialization_driver is a set.  Does that scale better?
    // Or, should we just wait for automatic control replication?
    std::vector<std::vector<PhaseBarrier>> phase_barriers(handles.size());
    std::vector<std::vector<std::set<int>>> master_colors(handles.size(), std::vector<std::set<int>>(num_ranks));
    for (int idx = 0; idx < handles.size(); idx++) {
      handle_t h = handles[idx];
      for (int master_color=0; master_color < num_ranks; ++master_color) {
        std::set<int> slave_colors;
        LegionRuntime::HighLevel::IndexSpace shared_subspace = runtime->get_index_subspace(ctx, h.shared_ip, master_color);
        LegionRuntime::HighLevel::IndexIterator shared_it(runtime, ctx, shared_subspace);
        while (shared_it.has_next()) {
          const ptr_t shared_ptr = shared_it.next();
          for (int slave_color = 0; slave_color < num_ranks; ++slave_color) {
            LegionRuntime::HighLevel::IndexSpace ghost_subspace = runtime->get_index_subspace(ctx, h.ghost_ip, slave_color);
            LegionRuntime::HighLevel::IndexIterator ghost_it(runtime, ctx, ghost_subspace);
            while (ghost_it.has_next()) {
              const ptr_t ghost_ptr = ghost_it.next();
              if (ghost_ptr == shared_ptr) {
                slave_colors.insert(slave_color);
                master_colors[idx][slave_color].insert(master_color);
              }
            } // while ghost_it
          } // for slave_color
        } // while shared_it
        phase_barriers[idx].push_back(runtime->create_phase_barrier(ctx, 1 + slave_colors.size()));
       } // for master_color
    } // for idx < handles.size()

    std::vector<Serializer> arg_map_serializer(num_ranks);

    spmd_task_args sargs;
    sargs.num_handles = handles.size();
    sargs.num_colors = num_ranks;

    for(size_t rank = 0; rank < num_ranks; ++rank){
      std::vector<PhaseBarrier> pbarriers_as_master;
      std::vector<size_t> num_masters;
      std::vector<std::vector<PhaseBarrier>> masters_pbarriers;
      for (int idx = 0; idx < handles.size(); idx++) {
        pbarriers_as_master.push_back(phase_barriers[idx][rank]);
        num_masters.push_back(master_colors[idx][rank].size());
        std::vector<PhaseBarrier> masters_pbs;
        for (std::set<int>::iterator master=master_colors[idx][rank].begin();
            master!=master_colors[idx][rank].end(); ++master)
          masters_pbs.push_back(phase_barriers[idx][*master]);
        masters_pbarriers.push_back(masters_pbs);
      } // for idx handles.size
      arg_map_serializer[rank].serialize(&sargs, sizeof(spmd_task_args));
      arg_map_serializer[rank].serialize(&pbarriers_as_master[0], handles.size() * sizeof(PhaseBarrier));
      arg_map_serializer[rank].serialize(&num_masters[0], handles.size() * sizeof(size_t));
      for (int idx = 0; idx < handles.size(); idx++)
        arg_map_serializer[rank].serialize(&masters_pbarriers[idx][0], num_masters[idx] * sizeof(PhaseBarrier));

      arg_map.set_point(Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::make_point(rank)),
        TaskArgument(arg_map_serializer[rank].get_buffer(), arg_map_serializer[rank].get_used_bytes()));
    } // for rank

    LegionRuntime::HighLevel::IndexLauncher spmd_launcher(
      task_ids_t::instance().spmd_task_id,
      LegionRuntime::HighLevel::Domain::from_rect<1>(
         context_.interop_helper_.all_processes_),
      TaskArgument(task_args_serializer.get_buffer(), task_args_serializer.get_used_bytes()), arg_map);
   
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    for (int idx = 0; idx < handles.size(); idx++) {
      handle_t h = handles[idx];

      LogicalPartition lp_excl = runtime->get_logical_partition(ctx, h.lr, h.exclusive_ip);
      spmd_launcher.add_region_requirement(
        RegionRequirement(lp_excl, 0 /*proj*/, READ_WRITE, EXCLUSIVE, h.lr));
      spmd_launcher.add_field((1 + num_ranks)*idx,fid_t.fid_value);

      LogicalPartition lp_shared = runtime->get_logical_partition(ctx, h.lr, h.shared_ip);
      //spmd_launcher.add_region_requirement(
      //  RegionRequirement(lp_shared, 0 /*proj*/, READ_WRITE, SIMULTANEOUS, h.lr));
      //spmd_launcher.add_field((2+num_ranks)*idx + 1,fid_t.fid_value);

      IndexSpace is_parent = runtime->get_parent_index_space(ctx, h.ghost_ip);
      for (size_t color = 0; color < num_ranks; color++) {
        LogicalRegion lr_potential_neighbor = runtime->get_logical_subregion_by_color(ctx, lp_shared, color);
        spmd_launcher.add_region_requirement(
          RegionRequirement(lr_potential_neighbor, READ_WRITE, SIMULTANEOUS, h.lr)
          .add_flags(NO_ACCESS_FLAG).add_field(fid_t.fid_value));

        IndexSpace ispace_ghost = runtime->get_index_subspace(ctx, h.ghost_ip, color);
        spmd_launcher.add_index_requirement(IndexSpaceRequirement(ispace_ghost, NO_MEMORY, is_parent));
      }

    }

    must_epoch_launcher.add_index_task(spmd_launcher);
 
    FutureMap fm = runtime->execute_must_epoch(ctx,must_epoch_launcher);
    fm.wait_all_results();


    //remove phase barriers:
    for (unsigned idx = 0; idx < phase_barriers.size(); idx++) {
      for (unsigned j = 0; j < phase_barriers[idx].size(); j++)
        runtime->destroy_phase_barrier(ctx, phase_barriers[idx][j]);
      phase_barriers[idx].clear();
    }
    phase_barriers.clear();


    // finish up legion runtime and handoff to mpi
    context_.interop_helper_.unset_call_mpi(ctx, runtime);
    context_.interop_helper_.handoff_to_mpi(ctx, runtime);

    // Set the current task context to the driver
    context_t::instance().pop_state(
      utils::const_string_t{"specialization_driver"}.hash());
} // mpilegion_runtime_driver



void
spmd_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{

  const int my_color = task->index_point.point_data[0];
  std::cout << "spmd " << my_color << std::endl;
  /*  context_t & context_ = context_t::instance();
  context_.push_state(utils::const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

  data_client dc;

  assert(task->arglen > 0);
  assert(task->local_arglen > 0);

  void* args_buf = task->args;
  void* local_args_buf = task->local_args;

  Deserializer args_deserializer(task->args, task->arglen);
  Deserializer local_args_deserializer(task->local_args,task->local_arglen);

  void* spmd_args_buf = malloc(sizeof(spmd_task_args));
  local_args_deserializer.deserialize(spmd_args_buf, sizeof(spmd_task_args));
  auto spmd_args = (spmd_task_args*)spmd_args_buf;

  const size_t num_handles = spmd_args->num_handles;
  const size_t num_colors = spmd_args->num_colors;

  assert(regions.size() == ((2 + num_colors) * num_handles));
  assert(task->regions.size() == ((2 + num_colors) * num_handles));

    void* handles_buf = malloc(sizeof(handle_t) * num_handles);
    args_deserializer.deserialize(handles_buf, sizeof(handle_t) * num_handles);

    void* hashes_buf = malloc(sizeof(size_t) * num_handles);
    args_deserializer.deserialize(hashes_buf, sizeof(size_t) * num_handles);

    void* namespaces_buf = malloc(sizeof(size_t) * num_handles);
    args_deserializer.deserialize(namespaces_buf, sizeof(size_t) * num_handles);

    void* versions_buf = malloc(sizeof(size_t) * num_handles);
    args_deserializer.deserialize(versions_buf, sizeof(size_t) * num_handles);

    PhaseBarrier* pbarriers_as_master = (PhaseBarrier*)malloc(sizeof(PhaseBarrier) * num_handles);
    local_args_deserializer.deserialize((void*)pbarriers_as_master, sizeof(PhaseBarrier) * num_handles);

    size_t* num_masters = (size_t*)malloc(sizeof(size_t) * num_handles);
    local_args_deserializer.deserialize((void*)num_masters, sizeof(size_t) * num_handles);

    Legion::LogicalRegion empty_lr;
    Legion::IndexPartition empty_ip;

    field_ids_t & fid_t =field_ids_t::instance();

    // fix handles on spmd side
    handle_t* fix_handles = (handle_t*)handles_buf;
    for (size_t idx = 0; idx < num_handles; idx++) {
      fix_handles[idx].pbarrier_as_master_ptr = &(pbarriers_as_master[idx]);

      PhaseBarrier* masters_pbarriers_buf = (PhaseBarrier*)malloc(sizeof(PhaseBarrier) * num_masters[idx]);
      local_args_deserializer.deserialize((void*)masters_pbarriers_buf, sizeof(PhaseBarrier) * num_masters[idx]);
      std::vector<PhaseBarrier> masters_pbarriers;
      for (size_t master = 0; master < num_masters[idx]; master++)
        fix_handles[idx].masters_pbarriers_ptrs.push_back(&(masters_pbarriers_buf[master]));
      assert(fix_handles[idx].masters_pbarriers_ptrs.size() == num_masters[idx]);

      fix_handles[idx].lr = empty_lr;
      fix_handles[idx].exclusive_ip = empty_ip;
      fix_handles[idx].shared_ip = empty_ip;
      fix_handles[idx].ghost_ip = empty_ip;
      fix_handles[idx].exclusive_lr = regions[(2 + num_colors)*idx].get_logical_region();
      fix_handles[idx].shared_lr = regions[(2 + num_colors)*idx+1].get_logical_region();
      fix_handles[idx].ghost_lr = regions[(2 + num_colors)*idx+1].get_logical_region(); // FIXME make a halo region from index space
    }

    runtime->unmap_all_regions(ctx);

    flecsi_put_all_handles(dc, dense, num_handles,
      (handle_t*)handles_buf,
      (size_t*)hashes_buf,
      (size_t*)namespaces_buf,
      (size_t*)versions_buf);



  // We obtain map of hashes to regions[n] here
  // We create halo LogicalRegions here
  // We might put all of this in the context_ for driver()

  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  clog(info) << "inside SPMD task, shard# = " << my_color  << std::endl;

  const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  int argc = args.argc + 1;
  char **argv;
  argv = (char**)std::malloc(sizeof(char*)*argc);
  std::memcpy(argv, args.argv, args.argc*sizeof(char*));
  argv[argc - 1] = (char*)&dc;

  driver(argc, argv);

  context_.pop_state(utils::const_string_t{"driver"}.hash());
*/
}


} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
