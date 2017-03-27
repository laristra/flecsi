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

    field_ids_t & fid_t =field_ids_t::instance();

    std::vector<handle_t> handles;
    std::vector<size_t> hashes;
    std::vector<size_t> namespaces;
    std::vector<size_t> versions;
    flecsi_get_all_handles(dc, dense, handles, hashes, namespaces, versions);
    
    //create phase barriers per handle for SPMD launch from partitions created and
    //registered to the data Client at the specialization_driver
    int num_colors;
    MPI_Comm_size(MPI_COMM_WORLD, &num_colors);

    // FIXME Will not scale. Coloring in serialization_driver is a set.  Does that scale better?
    // Or, should we just wait for automatic control replication?
    std::vector<std::vector<PhaseBarrier>> phase_barriers(handles.size());
    std::vector<std::vector<std::set<int>>> master_colors(handles.size(), std::vector<std::set<int>>(num_colors));
    for (int idx = 0; idx < handles.size(); idx++) {
      handle_t h = handles[idx];
      for (int master_color=0; master_color < num_colors; ++master_color) {
        std::set<int> slave_colors;
        LegionRuntime::HighLevel::IndexSpace shared_subspace = runtime->get_index_subspace(ctx, h.shared_ip, master_color);
        LegionRuntime::HighLevel::IndexIterator shared_it(runtime, ctx, shared_subspace);
        while (shared_it.has_next()) {
          const ptr_t shared_ptr = shared_it.next();
          for (int slave_color = 0; slave_color < num_colors; ++slave_color) {
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

    std::vector<Serializer> args_serializers(num_colors);

    spmd_task_args sargs;
    sargs.num_handles = handles.size();
    sargs.num_colors = num_colors;

    for(size_t color= 0; color < num_colors; ++color){
      std::vector<PhaseBarrier> pbarriers_as_master;
      std::vector<size_t> num_masters;
      std::vector<std::vector<PhaseBarrier>> masters_pbarriers;
      for (int idx = 0; idx < handles.size(); idx++) {
        pbarriers_as_master.push_back(phase_barriers[idx][color]);
        num_masters.push_back(master_colors[idx][color].size());
        std::vector<PhaseBarrier> masters_pbs;
        for (std::set<int>::iterator master=master_colors[idx][color].begin();
            master!=master_colors[idx][color].end(); ++master)
          masters_pbs.push_back(phase_barriers[idx][*master]);
        masters_pbarriers.push_back(masters_pbs);
      } // for idx handles.size
      args_serializers[color].serialize(&sargs, sizeof(spmd_task_args));
      args_serializers[color].serialize(&handles[0], handles.size() * sizeof(handle_t));
      args_serializers[color].serialize(&hashes[0], hashes.size() * sizeof(size_t));
      args_serializers[color].serialize(&namespaces[0], namespaces.size() * sizeof(size_t));
      args_serializers[color].serialize(&versions[0], versions.size() * sizeof(size_t));
      args_serializers[color].serialize(&pbarriers_as_master[0], handles.size() * sizeof(PhaseBarrier));
      args_serializers[color].serialize(&num_masters[0], handles.size() * sizeof(size_t));
      for (int idx = 0; idx < handles.size(); idx++)
        args_serializers[color].serialize(&masters_pbarriers[idx][0], num_masters[idx] * sizeof(PhaseBarrier));

      TaskLauncher spmd_launcher(task_ids_t::instance().spmd_task_id,
          TaskArgument(args_serializers[color].get_buffer(), args_serializers[color].get_used_bytes()));

      spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;
   

      for (int idx = 0; idx < handles.size(); idx++) {
        handle_t h = handles[idx];

        LogicalPartition lp_excl = runtime->get_logical_partition(ctx, h.lr, h.exclusive_ip);
        LogicalRegion lr_excl = runtime->get_logical_subregion_by_color(ctx, lp_excl, color);
        spmd_launcher.add_region_requirement(
          RegionRequirement(lr_excl, READ_WRITE, EXCLUSIVE, h.lr).add_field(fid_t.fid_value));

        LogicalPartition lp_shared = runtime->get_logical_partition(ctx, h.lr, h.shared_ip);
        LogicalRegion lr_shared = runtime->get_logical_subregion_by_color(ctx, lp_shared, color);
        spmd_launcher.add_region_requirement(
          RegionRequirement(lr_shared, READ_WRITE, SIMULTANEOUS, h.lr).add_field(fid_t.fid_value));

        for (std::set<int>::iterator master=master_colors[idx][color].begin();
            master!=master_colors[idx][color].end(); ++master) {
          LogicalRegion lregion_ghost = runtime->get_logical_subregion_by_color(ctx,
              lp_shared,*master);
          spmd_launcher.add_region_requirement(
            RegionRequirement(lregion_ghost, READ_ONLY, SIMULTANEOUS, h.lr)
            .add_flags(NO_ACCESS_FLAG).add_field(fid_t.fid_value));
        }

        IndexSpace is_parent = runtime->get_parent_index_space(ctx, h.ghost_ip);
        IndexSpace ispace_ghost = runtime->get_index_subspace(ctx, h.ghost_ip, color);
        spmd_launcher.add_index_requirement(IndexSpaceRequirement(ispace_ghost, NO_MEMORY, is_parent));
      } // for handles.size()

      DomainPoint point(color);
      must_epoch_launcher.add_single_task(point,spmd_launcher);

    } // for rank

 
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

  context_t & context_ = context_t::instance();
  context_.push_state(utils::const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

  data_client dc;

  assert(task->arglen > 0);

  void* args_buf = task->args;

  Deserializer args_deserializer(task->args, task->arglen);

  void* spmd_args_buf = malloc(sizeof(spmd_task_args));
  args_deserializer.deserialize(spmd_args_buf, sizeof(spmd_task_args));
  auto spmd_args = (spmd_task_args*)spmd_args_buf;

  const size_t num_handles = spmd_args->num_handles;
  //FIXME remove const size_t num_colors = spmd_args->num_colors;

  assert(regions.size() >= (2 * num_handles));
  assert(task->regions.size() >= (2 * num_handles));
  assert(task->indexes.size() == num_handles);


  void* handles_buf = malloc(sizeof(handle_t) * num_handles);
  args_deserializer.deserialize(handles_buf, sizeof(handle_t) * num_handles);

  void* hashes_buf = malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize(hashes_buf, sizeof(size_t) * num_handles);

  void* namespaces_buf = malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize(namespaces_buf, sizeof(size_t) * num_handles);

  void* versions_buf = malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize(versions_buf, sizeof(size_t) * num_handles);

  PhaseBarrier* pbarriers_as_master = (PhaseBarrier*)malloc(sizeof(PhaseBarrier) * num_handles);
  args_deserializer.deserialize((void*)pbarriers_as_master, sizeof(PhaseBarrier) * num_handles);

  size_t* num_masters = (size_t*)malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize((void*)num_masters, sizeof(size_t) * num_handles);

  Legion::LogicalRegion empty_lr;
  Legion::IndexPartition empty_ip;

  std::vector<LogicalRegion>  lregions_ghost(num_handles);

  // fix handles on spmd side
  handle_t* fix_handles = (handle_t*)handles_buf;
  int region_index = 0;
  for (size_t idx = 0; idx < num_handles; idx++) {
    fix_handles[idx].pbarrier_as_master_ptr = &(pbarriers_as_master[idx]);

    PhaseBarrier* masters_pbarriers_buf = (PhaseBarrier*)malloc(sizeof(PhaseBarrier) * num_masters[idx]);
    args_deserializer.deserialize((void*)masters_pbarriers_buf, sizeof(PhaseBarrier) * num_masters[idx]);
    std::vector<PhaseBarrier> masters_pbarriers;
    for (size_t master = 0; master < num_masters[idx]; master++)
      fix_handles[idx].masters_pbarriers_ptrs.push_back(&(masters_pbarriers_buf[master]));
    assert(fix_handles[idx].masters_pbarriers_ptrs.size() == num_masters[idx]);

    fix_handles[idx].lr = empty_lr;
    fix_handles[idx].exclusive_ip = empty_ip;
    fix_handles[idx].shared_ip = empty_ip;
    fix_handles[idx].ghost_ip = empty_ip;
    fix_handles[idx].exclusive_lr = regions[region_index++].get_logical_region();
    fix_handles[idx].shared_lr = regions[region_index++].get_logical_region();
    for (size_t master = 0; master < num_masters[idx]; master++)
      fix_handles[idx].pregions_neighbors_shared.push_back(regions[region_index++]);

    FieldSpace fspace_ghost = fix_handles[idx].shared_lr.get_field_space();
    IndexSpace ispace_ghost = task->indexes[idx].handle;
    lregions_ghost[idx] = runtime->create_logical_region(ctx, ispace_ghost, fspace_ghost);
    char buf[40];
    sprintf(buf,"spmd %d lregion_ghost %d", my_color, idx);
    runtime->attach_name(lregions_ghost[idx], buf);

    fix_handles[idx].ghost_lr = lregions_ghost[idx];
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

  //remove ghost logical regions
  for (unsigned idx = 0; idx < num_handles; idx++)
    runtime->destroy_logical_region(ctx, lregions_ghost[idx]);
  lregions_ghost.clear();

  context_.pop_state(utils::const_string_t{"driver"}.hash());

}


} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
