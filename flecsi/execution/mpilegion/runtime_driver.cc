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
      size_t buf_size;
      size_t num_handles;
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

    // ndm - create data client here
    data_client dc;
    int argc = args.argc + 1;
    char **argv;
    argv = (char**)std::malloc(sizeof(char*)*argc);
    std::memcpy(argv, args.argv, args.argc*sizeof(char*));
    argv[argc - 1] = (char*)&dc;

    // run default or user-defined specialization driver 
    specialization_driver(argc, argv);

    //creating phace barriers for SPMD launch from partitions created and 
    //registered to the data Client at the specialization_driver 
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    std::vector<PhaseBarrier> phase_barriers;
    std::vector<std::set<int>> master_colors(num_ranks);
/*    for (int master_color=0; master_color < num_ranks; ++master_color) {
      std::set<int> slave_colors;
      for (std::set<ptr_t>::iterator
         it = cells_shared_coloring[master_color].points.begin();
         it != cells_shared_coloring[master_color].points.end(); ++it) {
         
         const ptr_t ptr = *it;
         for (int slave_color = 0; slave_color < num_ranks; ++slave_color)
           if (cells_ghost_coloring[slave_color].points.count(ptr)) {
             slave_colors.insert(slave_color);
             master_colors[slave_color].insert(master_color);
          }
       }
    phase_barriers.push_back(runtime->create_phase_barrier(ctx,
      1 + slave_colors.size()));
   }
*/
   //FIXME use Legion's serialized instead SPMDArgsSerializer
/*
   std::vector<execution::sprint::SPMDArgs> spmd_args(num_ranks);
   std::vector<execution::sprint::SPMDArgsSerializer>
           args_seriliazed(num_ranks);
   for (int color=0; color < num_ranks; ++color) {
      spmd_args[color].pbarrier_as_master = phase_barriers[color];
      for (std::set<int>::iterator master=master_colors[color].begin();
              master!=master_colors[color].end(); ++master)
        spmd_args[color].masters_pbarriers.push_back(phase_barriers[*master]);
        args_seriliazed[color].archive(&(spmd_args[color]));
        arg_map.set_point(DomainPoint::from_point<1>(
            LegionRuntime::Arrays::Point<1>(color)),
            TaskArgument(args_seriliazed[color].getBitStream(),
            args_seriliazed[color].getBitStreamSize()));
  }
*/
    //execute SPMD launch that execute user-defined driver



    MustEpochLauncher must_epoch_launcher; 
    LegionRuntime::HighLevel::ArgumentMap arg_map;

    // ndm - need to be able to get data handle here
    field_ids_t & fid_t =field_ids_t::instance();

    std::vector<handle_t> handles;
    std::vector<size_t> hashes;
    std::vector<size_t> namespaces;
    std::vector<size_t> versions;
    flecsi_get_all_handles(dc, dense, handles, hashes, namespaces, versions);
    
    Serializer serializer;
    serializer.serialize(&handles[0], handles.size() * sizeof(handle_t)); 
    serializer.serialize(&hashes[0], hashes.size() * sizeof(size_t)); 
    serializer.serialize(&namespaces[0], namespaces.size() * sizeof(size_t)); 
    serializer.serialize(&versions[0], versions.size() * sizeof(size_t)); 

    const void* args_buf = serializer.get_buffer();

    spmd_task_args sargs;
    sargs.buf_size = serializer.get_used_bytes();
    sargs.num_handles = handles.size();

    for(size_t i = 0; i < num_ranks; ++i){
      arg_map.set_point(Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::make_point(i)),
        TaskArgument(&sargs, sizeof(sargs)));
    }

    LegionRuntime::HighLevel::IndexLauncher spmd_launcher(
      task_ids_t::instance().spmd_task_id,
      LegionRuntime::HighLevel::Domain::from_rect<1>(
         context_.interop_helper_.all_processes_),
      TaskArgument(args_buf, sargs.buf_size), arg_map);
   
    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    for (int idx = 0; idx < handles.size(); idx++) {
      handle_t h = handles[idx];

      LogicalPartition lp_excl = runtime->get_logical_partition(ctx, h.lr, h.exclusive_ip);
      spmd_launcher.add_region_requirement(
        RegionRequirement(lp_excl, 0 /*proj*/, READ_WRITE, EXCLUSIVE, h.lr));
      spmd_launcher.add_field(3*idx,fid_t.fid_value);

      // FIXME  this is temporary for verifying 1st data movement - this will be RW, SIMUL
      LogicalPartition lp_shared = runtime->get_logical_partition(ctx, h.lr, h.shared_ip);
      spmd_launcher.add_region_requirement(
        RegionRequirement(lp_shared, 0 /*proj*/, READ_ONLY, EXCLUSIVE, h.lr));
      spmd_launcher.add_field(3*idx + 1,fid_t.fid_value);

      // FIXME  this is temporary for verifying 1st data movement - this will be RO, SIMUL for each neighbors' shared and pass ghost_ip as IndexSpace
      LogicalPartition lp_ghost = runtime->get_logical_partition(ctx, h.lr, h.ghost_ip);
      spmd_launcher.add_region_requirement(
        RegionRequirement(lp_ghost, 0 /*proj*/, READ_ONLY, EXCLUSIVE, h.lr));
      spmd_launcher.add_field(3*idx +2,fid_t.fid_value);

      // jpg - do neighbors shared and phase barriers here
    }

    // PAIR_PROGRAMMING
    // This is where we iterate over data and spaces from specialization_driver().
    // We create RegionRequirements here
    // We serialize a map (if it is not in MPI-context)
    // We serialize phase barriers
    must_epoch_launcher.add_index_task(spmd_launcher);
 
    FutureMap fm = runtime->execute_must_epoch(ctx,must_epoch_launcher);
    fm.wait_all_results();

//FIXME add region requirements for logical partitions from data client
#if 0
    spmd_launcher.add_region_requirement(
      RegionRequirement(cells_shared_lp, 0/*projection ID*/,
                    READ_ONLY, SIMULTANEOUS, cells_lr));
    spmd_launcher.add_field(0, fid_t.fid_data);
    spmd_launcher.add_region_requirement(
       RegionRequirement(cells_ghost_lp, 0/*projection ID*/,
                    READ_ONLY, SIMULTANEOUS, cells_lr));
    spmd_launcher.add_field(1, fid_t.fid_data);
#endif


    //remove phase barriers:

    for (unsigned idx = 0; idx < phase_barriers.size(); idx++)
       runtime->destroy_phase_barrier(ctx, phase_barriers[idx]);
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

  // PAIR_PROGRAMMING
  if (task->arglen > 0) {
    void* args_buf = task->args;
    auto args = (spmd_task_args*)task->local_args;

    size_t num_handles = args->num_handles;

    assert(regions.size() == 3*num_handles);
    assert(task->regions.size() == 3*num_handles);

    Deserializer deserializer(args_buf, args->buf_size);

    void* handles_buf = malloc(sizeof(handle_t) * num_handles);
    deserializer.deserialize(handles_buf, sizeof(handle_t) * num_handles);

    void* hashes_buf = malloc(sizeof(size_t) * num_handles);
    deserializer.deserialize(hashes_buf, sizeof(size_t) * num_handles);

    void* namespaces_buf = malloc(sizeof(size_t) * num_handles);
    deserializer.deserialize(namespaces_buf, sizeof(size_t) * num_handles);

    void* versions_buf = malloc(sizeof(size_t) * num_handles);
    deserializer.deserialize(versions_buf, sizeof(size_t) * num_handles);

    Legion::LogicalRegion empty_lr;
    Legion::IndexPartition empty_ip;

    field_ids_t & fid_t =field_ids_t::instance();

    // fix handles on spmd side
    handle_t* fix_handles = (handle_t*)handles_buf;
    for (size_t idx = 0; idx < num_handles; idx++) {

      {
        LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic,size_t> acc_legion =
        regions[3*idx].get_field_accessor(fid_t.fid_value).typeify<size_t>();

        Domain dom = runtime->get_index_space_domain(ctx,
              task->regions[3*idx].region.get_index_space());
          Rect<1> rect = dom.get_rect<1>();
          for (GenericPointInRectIterator<1> pir(rect); pir; pir++) {
            std::cout << my_color <<"exclusive " << DomainPoint::from_point<1>(pir.p)
                << " = " << acc_legion.read(DomainPoint::from_point<1>(pir.p)) << std::endl;
          }
      }

      {
        LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic,size_t> acc_legion =
        regions[3*idx+1].get_field_accessor(fid_t.fid_value).typeify<size_t>();

        Domain dom = runtime->get_index_space_domain(ctx,
              task->regions[3*idx+1].region.get_index_space());
          Rect<1> rect = dom.get_rect<1>();
          for (GenericPointInRectIterator<1> pir(rect); pir; pir++) {
            std::cout << my_color <<"shared " << DomainPoint::from_point<1>(pir.p)
                << " = " << acc_legion.read(DomainPoint::from_point<1>(pir.p)) << std::endl;
          }
      }

      {
        LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic,size_t> acc_legion =
        regions[3*idx+2].get_field_accessor(fid_t.fid_value).typeify<size_t>();

        Domain dom = runtime->get_index_space_domain(ctx,
              task->regions[3*idx+2].region.get_index_space());
          Rect<1> rect = dom.get_rect<1>();
          for (GenericPointInRectIterator<1> pir(rect); pir; pir++) {
            std::cout << my_color <<"ghost " << DomainPoint::from_point<1>(pir.p)
                << " = " << acc_legion.read(DomainPoint::from_point<1>(pir.p)) << std::endl;
          }
      }



      fix_handles[idx].lr = empty_lr;
      fix_handles[idx].exclusive_ip = empty_ip;
      fix_handles[idx].shared_ip = empty_ip;
      fix_handles[idx].ghost_ip = empty_ip;
      fix_handles[idx].exclusive_lr = regions[3*idx].get_logical_region();
      runtime->unmap_region(ctx, regions[3*idx]);
      fix_handles[idx].shared_lr = regions[3*idx+1].get_logical_region();
      runtime->unmap_region(ctx, regions[3*idx+1]);
      fix_handles[idx].ghost_lr = regions[3*idx+2].get_logical_region();  
      runtime->unmap_region(ctx, regions[3*idx+2]);
    }

    flecsi_put_all_handles(dc, dense, num_handles,
      (handle_t*)handles_buf,
      (size_t*)hashes_buf,
      (size_t*)namespaces_buf,
      (size_t*)versions_buf);
  }
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
}


} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
