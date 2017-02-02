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
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#include <iostream>

#include "flecsi/execution/context.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/execution/mpilegion/lax_wendroff_task.h"

#include "flecsi/execution/legion/dpd.h"

namespace flecsi {
namespace execution {
namespace lax_wendroff {

void
lax_halo_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
    using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
    using field_id = LegionRuntime::HighLevel::FieldID;

    assert(regions.size() == 2);
    assert(task->regions.size() == 2);
    assert(task->regions[0].privilege_fields.size() == 1);
    assert(task->regions[1].privilege_fields.size() == 1);

    field_id fid = *(task->regions[0].privilege_fields.begin());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_shared= regions[0].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());
    std::set<ptr_t> shared_pts;  // TODO profile this or switch to dense storage
    while(itr_shared.has_next())
    	shared_pts.insert(itr_shared.next());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_ghost= regions[1].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_ghost(runtime, ctx, regions[1].get_logical_region());
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      if (shared_pts.count(ptr))
    	  acc_ghost.write(ptr, acc_shared.read(ptr));
    }
}

void
lax_wendroff_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(task->indexes.size() == 1);
  assert(regions.size() >= 2);
  assert(task->regions.size() >= 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);

  field_id fid = *(task->regions[0].privilege_fields.begin());

  const int my_rank = task->index_point.get_index();

  sprint::SPMDArgs args;
  sprint::SPMDArgsSerializer args_serializer;
  args_serializer.setBitStream(task->args);
  args_serializer.restore(&args);

  LogicalRegion lr_shared = regions[0].get_logical_region();
  runtime->unmap_region(ctx, regions[0]);

  //LegionRuntime::HighLevel::LogicalRegion lr_exclusive = regions[1].get_logical_region();

  std::vector<LogicalRegion> lregions_ghost;
  for (int index = 2; index < regions.size(); index++) {
	  LogicalRegion lr_ghost = regions[index].get_logical_region();
	  lregions_ghost.push_back(lr_ghost);
	  runtime->unmap_region(ctx, regions[index]);
  }

  FieldSpace fspace_halo = runtime->create_field_space(ctx);
  {
  	char buf[40];
  	sprintf(buf,"spmd fspace_halo %d",my_rank);
  	runtime->attach_name(fspace_halo, buf);
  	FieldAllocator allocator = runtime->create_field_allocator(ctx, fspace_halo);
  	allocator.allocate_field(sizeof(size_t), fid);
  }
  IndexSpace ispace_halo = task->indexes[0].handle;
  LogicalRegion lregion_halo = runtime->create_logical_region(ctx, ispace_halo, fspace_halo);
  {
  	char buf[40];
  	sprintf(buf,"spmd lregion_halo %d",my_rank);
  	runtime->attach_name(lregion_halo, buf);
  }

  for (int cycle = 0; cycle < 2; cycle++) {

    // phase 1 masters update their halo regions; slaves may not access data

    // as master

    {
      args.pbarrier_as_master.wait();                     // phase 1
      // master writes to data
      std::cout << my_rank << " as master writes data; phase 1 of cycle " << cycle << std::endl;
      RegionRequirement shared_req(lr_shared, READ_WRITE, EXCLUSIVE,
      		lr_shared);
      shared_req.add_field(fid);
      InlineLauncher shared_launcher(shared_req);
      PhysicalRegion pregion_shared = runtime->map_region(ctx, shared_launcher);
      LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
        acc_shared= pregion_shared.get_field_accessor(fid).typeify<size_t>();
      IndexIterator itr_shared(runtime, ctx, lr_shared);
      while(itr_shared.has_next()){
        ptr_t ptr = itr_shared.next();
        acc_shared.write(ptr, static_cast<size_t>(ptr.value + cycle));
      }
      runtime->unmap_region(ctx, pregion_shared);

      args.pbarrier_as_master.arrive(1);                  // phase 2
      args.pbarrier_as_master =
            runtime->advance_phase_barrier(ctx, args.pbarrier_as_master);             // phase 2
    }

    // as slave

    for (int master=0; master < args.masters_pbarriers.size(); master++) {
        args.masters_pbarriers[master].arrive(1);                                     // phase 2
        args.masters_pbarriers[master] =
                runtime->advance_phase_barrier(ctx, args.masters_pbarriers[master]);  // phase 2
    }

    // phase 2 slaves can read data; masters may not write to data

    // as master

    args.pbarrier_as_master.arrive(1);                                                // phase cycle + 1
    args.pbarrier_as_master =
            runtime->advance_phase_barrier(ctx, args.pbarrier_as_master);             // phase cycle + 1

    // as slave

      for (int master=0; master < args.masters_pbarriers.size(); master++) {
          AcquireLauncher acquire_launcher(lregions_ghost[master], lregions_ghost[master],
        		  regions[2+master]);
          acquire_launcher.add_field(fid);
          acquire_launcher.add_wait_barrier(args.masters_pbarriers[master]);            // phase 2
          runtime->issue_acquire(ctx, acquire_launcher);


    	  TaskLauncher launcher(task_ids_t::instance().lax_halo_task_id, TaskArgument(nullptr, 0));
    	  launcher.add_region_requirement(RegionRequirement(lregions_ghost[master], READ_ONLY, EXCLUSIVE,
    			  lregions_ghost[master]));
    	  launcher.add_field(0, fid);
    	  launcher.add_region_requirement(RegionRequirement(lregion_halo, READ_WRITE,
                  EXCLUSIVE, lregion_halo));
    	  launcher.add_field(1, fid);
          runtime->execute_task(ctx, launcher);

          // slave reads data
          std::cout << my_rank << " as slave copies data from " << master << ", cycle " << cycle << std::endl;

          ReleaseLauncher release_launcher(lregions_ghost[master], lregions_ghost[master],
        		  regions[2+master]);
          release_launcher.add_field(fid);
    	  release_launcher.add_arrival_barrier(args.masters_pbarriers[master]);         // phase cycle + 1
    	  runtime->issue_release(ctx, release_launcher);

          args.masters_pbarriers[master] =
          		runtime->advance_phase_barrier(ctx, args.masters_pbarriers[master]);  // phase cycle + 1
      } // for master as slave

      RegionRequirement halo_req(lregion_halo, READ_ONLY, EXCLUSIVE,
      		lregion_halo);
      halo_req.add_field(fid);
      InlineLauncher halo_launcher(halo_req);
      PhysicalRegion pregion_halo = runtime->map_region(ctx, halo_launcher);
      LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
        acc_halo= pregion_halo.get_field_accessor(fid).typeify<size_t>();
      IndexIterator itr_halo(runtime, ctx, ispace_halo);
      while(itr_halo.has_next()){
        ptr_t ptr = itr_halo.next();
        assert(acc_halo.read(ptr) == static_cast<size_t>(ptr.value + cycle));
      }
      runtime->unmap_region(ctx, pregion_halo);

  } // cycle


  std::cout << "test ghost access ... passed"
  << std::endl;
}//ghost_access_task

} // namespace lax_wendroff
} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

