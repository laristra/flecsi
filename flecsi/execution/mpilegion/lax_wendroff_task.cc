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
#include <fstream>

#include "flecsi/execution/context.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/execution/mpilegion/lax_wendroff_task.h"

#define NX 64
#define NY 64
#define CFL 0.5
#define U 1.0
#define V 1.0

namespace flecsi {
namespace execution {
namespace lax_wendroff {

static std::map<size_t, ptr_t> create_map (Legion::HighLevelRuntime *runtime,
	Legion::Context ctx,
	const Legion::PhysicalRegion &region)
{  // TODO profile this
  std::map<size_t, ptr_t> map;
  field_ids_t & fid_t =field_ids_t::instance();
  LegionRuntime::HighLevel::FieldID fid_gid = fid_t.fid_cell;
  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>
  	  acc = region.get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator itr(runtime, ctx, region.get_logical_region());
  while(itr.has_next()) {
    ptr_t ptr = itr.next();
    map[acc.read(ptr)] = ptr;
  }
  return map;
}

static void calc_x_indices(const ptr_t ptr,
		  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>& acc,
		  int* gid_plus_i, int* gid_minus_i)
{
  const int gid_pt = acc.read(ptr);
  const int gid_y_index = gid_pt / NX;
  const int gid_x_index = gid_pt % NX;
  *gid_plus_i = (gid_x_index + 1) != NX ? gid_x_index + 1 + gid_y_index * NX: -1;
  *gid_minus_i = gid_x_index != 0 ? gid_x_index - 1 + gid_y_index * NX: -1;
}

static void calc_y_indices(const ptr_t ptr,
		  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>& acc,
		  int* gid_plus_j, int* gid_minus_j)
{
  const int gid_pt = acc.read(ptr);
  const int gid_y_index = gid_pt / NX;
  const int gid_x_index = gid_pt % NX;
  *gid_plus_j = (gid_y_index + 1) != NY ? gid_x_index + (1 + gid_y_index) * NX: -1;
  *gid_minus_j = gid_y_index != 0 ? gid_x_index + (gid_y_index - 1) * NX: -1;
}

static double get_x_vel() {
  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  return U * dt / dx;
}

static double get_y_vel() {
  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  return V * dt / dy;
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
  assert(regions.size() >= 3);
  assert(task->regions.size() >= 3);
  assert(task->regions[0].privilege_fields.size() == 2);
  assert(task->regions[1].privilege_fields.size() == 2);
  assert(task->regions[2].privilege_fields.size() == 1);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  const int my_rank = task->index_point.get_index();

  sprint::SPMDArgs args;
  sprint::SPMDArgsSerializer args_serializer;
  args_serializer.setBitStream(task->args);
  args_serializer.restore(&args);

  LegionRuntime::HighLevel::LogicalRegion lr_exclusive = regions[1].get_logical_region();
  FieldSpace fspace = runtime->create_field_space(ctx);
  {
  	char buf[40];
  	sprintf(buf,"spmd fspace_halo %d",my_rank);
  	runtime->attach_name(fspace, buf);
  	FieldAllocator allocator = runtime->create_field_allocator(ctx, fspace);
  	allocator.allocate_field(sizeof(double), fid_phi);
  }
  LogicalRegion lr_excl_tmp = runtime->create_logical_region(ctx, lr_exclusive.get_index_space(), fspace);
  {
  	char buf[40];
  	sprintf(buf,"spmd lr_excl_tmp %d",my_rank);
  	runtime->attach_name(lr_excl_tmp, buf);
  }

  LogicalRegion lr_shared = regions[0].get_logical_region();
  runtime->unmap_region(ctx, regions[0]);
  LogicalRegion lr_shared_tmp = runtime->create_logical_region(ctx, lr_shared.get_index_space(), fspace);
  {
  	char buf[40];
  	sprintf(buf,"spmd lr_shared_tmp %d",my_rank);
  	runtime->attach_name(lr_shared_tmp, buf);
  }

  LogicalRegion lr_ghost = regions[2].get_logical_region();
  runtime->unmap_region(ctx, regions[2]);

  std::vector<LogicalRegion> lregions_ghost;
  std::vector<PhysicalRegion> pregions_ghost;
  for (int index = 3; index < regions.size(); index++) {
	  LogicalRegion lr_ghost = regions[index].get_logical_region();
	  lregions_ghost.push_back(lr_ghost);
	  pregions_ghost.push_back(regions[index]);
	  runtime->unmap_region(ctx, regions[index]);
  }

  IndexSpace ispace_halo = task->indexes[0].handle;
  LogicalRegion lregion_halo = runtime->create_logical_region(ctx, ispace_halo, fspace);
  {
  	char buf[40];
  	sprintf(buf,"spmd lregion_halo %d",my_rank);
  	runtime->attach_name(lregion_halo, buf);
  }

  sprint::TaskWrapper task_wrapper(&args, lregions_ghost, pregions_ghost, lregion_halo, task_ids_t::instance().double_copy_task_id, fid_phi);

  bool read_phase = false;
  bool write_phase = true;

  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  double time = 0.0;
  while (time < 0.165) {
    time += dt;
    std::cout << "t=" << time << std::endl;
    for (int split = 0; split < 2; split ++) {


    } // split
  } // cycle

  std::cout << "time " << time << std::endl;
}//ghost_access_task

} // namespace lax_wendroff
} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

