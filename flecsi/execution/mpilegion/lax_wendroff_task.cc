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
lax_adv_y_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using double_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, double>;
  using size_t_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 6);
  assert(task->regions.size() == 6);
  assert(task->regions[0].privilege_fields.size() == 2);
  assert(task->regions[1].privilege_fields.size() == 2);
  assert(task->regions[2].privilege_fields.size() == 1);
  assert(task->regions[3].privilege_fields.size() == 1);
  assert(task->regions[4].privilege_fields.size() == 1);
  assert(task->regions[5].privilege_fields.size() == 1);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  assert(task->arglen == sizeof(int));
  int my_rank = *(const int*)task->args;

  const double b = get_y_vel();

  double_acc_t acc_shared_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();
  size_t_acc_t acc_shared_gid = regions[0].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());

  double_acc_t acc_exclsv_phi = regions[1].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_ghost_phi = regions[2].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_shared_tmp_phi = regions[3].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator itr_shared_tmp(runtime, ctx, regions[3].get_logical_region());

  double_acc_t acc_excl_tmp_phi = regions[4].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator itr_excl_tmp(runtime, ctx, regions[4].get_logical_region());

  std::map<size_t, ptr_t> shared_map = create_map(runtime, ctx, regions[0]);
  std::map<size_t, ptr_t> excl_map = create_map(runtime, ctx, regions[1]);
  std::map<size_t, ptr_t> ghost_map = create_map(runtime, ctx, regions[5]);

  while(itr_shared.has_next()) {
    const ptr_t ptr = itr_shared.next();
    int gid_plus_j, gid_minus_j;
    calc_y_indices(ptr, acc_shared_gid, &gid_plus_j, &gid_minus_j);

	double value = -b * b * acc_shared_phi.read(ptr);

    if (shared_map.find(gid_plus_j) != shared_map.end())
        value += 0.5 * (b * b - b) * acc_shared_phi.read(shared_map.find(gid_plus_j)->second);
    else if (ghost_map.find(gid_plus_j) != ghost_map.end())
        value += 0.5 * (b * b - b) * acc_ghost_phi.read(ghost_map.find(gid_plus_j)->second);
    else if (excl_map.find(gid_plus_j) != excl_map.end())
        value += 0.5 * (b * b - b) * acc_exclsv_phi.read(excl_map.find(gid_plus_j)->second);

    if (shared_map.find(gid_minus_j) != shared_map.end())
        value += 0.5 * (b * b + b) * acc_shared_phi.read(shared_map.find(gid_minus_j)->second);
    else if (ghost_map.find(gid_minus_j) != ghost_map.end())
        value += 0.5 * (b * b + b) * acc_ghost_phi.read(ghost_map.find(gid_minus_j)->second);
    else if (excl_map.find(gid_minus_j) != excl_map.end())
        value += 0.5 * (b * b + b) * acc_exclsv_phi.read(excl_map.find(gid_minus_j)->second);
    acc_shared_tmp_phi.write(ptr, value);
  }

  while(itr_shared_tmp.has_next()) {
      const ptr_t ptr = itr_shared_tmp.next();
      acc_shared_phi.write(ptr, acc_shared_phi.read(ptr) + acc_shared_tmp_phi.read(ptr));
  }

  while(itr_excl_tmp.has_next()) {
      const ptr_t ptr = itr_excl_tmp.next();
      acc_exclsv_phi.write(ptr, acc_exclsv_phi.read(ptr) + acc_excl_tmp_phi.read(ptr));
  }
}

void
lax_calc_excl_y_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using double_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, double>;
  using size_t_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 3);
  assert(task->regions.size() == 3);
  assert(task->regions[0].privilege_fields.size() == 2);
  assert(task->regions[1].privilege_fields.size() == 2);
  assert(task->regions[2].privilege_fields.size() == 1);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  const double b = get_y_vel();

  double_acc_t acc_shared_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_exclsv_phi = regions[1].get_field_accessor(fid_phi).typeify<double>();
  size_t_acc_t acc_exclsv_gid = regions[1].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator exclsv_itr(runtime, ctx, regions[1].get_logical_region());

  double_acc_t acc_tmp_phi = regions[2].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator tmp_itr(runtime, ctx, regions[2].get_logical_region());

  std::map<size_t, ptr_t> shared_map = create_map(runtime, ctx, regions[0]);
  std::map<size_t, ptr_t> excl_map = create_map(runtime, ctx, regions[1]);

  while(exclsv_itr.has_next()) {
    const ptr_t ptr = exclsv_itr.next();
    int gid_plus_j, gid_minus_j;
    calc_y_indices(ptr, acc_exclsv_gid, &gid_plus_j, &gid_minus_j);

    double value = -b * b * acc_exclsv_phi.read(ptr);

    if (excl_map.find(gid_plus_j) != excl_map.end())
        value += 0.5 * (b * b - b) * acc_exclsv_phi.read(excl_map.find(gid_plus_j)->second);
    else if (shared_map.find(gid_plus_j) != shared_map.end())
        value += 0.5 * (b * b - b) * acc_shared_phi.read(shared_map.find(gid_plus_j)->second);

    if (excl_map.find(gid_minus_j) != excl_map.end())
        value += 0.5 * (b * b + b) * acc_exclsv_phi.read(excl_map.find(gid_minus_j)->second);
    else if (shared_map.find(gid_minus_j) != shared_map.end())
        value += 0.5 * (b * b + b) * acc_shared_phi.read(shared_map.find(gid_minus_j)->second);

    acc_tmp_phi.write(ptr, value);
  }

}

void
lax_adv_x_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using double_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, double>;
  using size_t_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 6);
  assert(task->regions.size() == 6);
  assert(task->regions[0].privilege_fields.size() == 2);
  assert(task->regions[1].privilege_fields.size() == 2);
  assert(task->regions[2].privilege_fields.size() == 1);
  assert(task->regions[3].privilege_fields.size() == 1);
  assert(task->regions[4].privilege_fields.size() == 1);
  assert(task->regions[5].privilege_fields.size() == 1);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  assert(task->arglen == sizeof(int));
  int my_rank = *(const int*)task->args;

  const double a = get_x_vel();

  double_acc_t acc_shared_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();
  size_t_acc_t acc_shared_gid = regions[0].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());

  double_acc_t acc_exclsv_phi = regions[1].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_ghost_phi = regions[2].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_shared_tmp_phi = regions[3].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator itr_shared_tmp(runtime, ctx, regions[3].get_logical_region());

  double_acc_t acc_excl_tmp_phi = regions[4].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator itr_excl_tmp(runtime, ctx, regions[4].get_logical_region());

  std::map<size_t, ptr_t> shared_map = create_map(runtime, ctx, regions[0]);
  std::map<size_t, ptr_t> excl_map = create_map(runtime, ctx, regions[1]);
  std::map<size_t, ptr_t> ghost_map = create_map(runtime, ctx, regions[5]);

  while(itr_shared.has_next()) {
    const ptr_t ptr = itr_shared.next();
    int gid_plus_i, gid_minus_i;
    calc_x_indices(ptr, acc_shared_gid, &gid_plus_i, &gid_minus_i);

    double value = -a * a * acc_shared_phi.read(ptr);

    if (shared_map.find(gid_plus_i) != shared_map.end())
        value += 0.5 * (a * a - a) * acc_shared_phi.read(shared_map.find(gid_plus_i)->second);
    else if (ghost_map.find(gid_plus_i) != ghost_map.end())
        value += 0.5 * (a * a - a) * acc_ghost_phi.read(ghost_map.find(gid_plus_i)->second);
    else if (excl_map.find(gid_plus_i) != excl_map.end())
        value += 0.5 * (a * a - a) * acc_exclsv_phi.read(excl_map.find(gid_plus_i)->second);

    if (shared_map.find(gid_minus_i) != shared_map.end())
        value += 0.5 * (a * a + a) * acc_shared_phi.read(shared_map.find(gid_minus_i)->second);
    else if (ghost_map.find(gid_minus_i) != ghost_map.end())
        value += 0.5 * (a * a + a) * acc_ghost_phi.read(ghost_map.find(gid_minus_i)->second);
    else if (excl_map.find(gid_minus_i) != excl_map.end())
        value += 0.5 * (a * a + a) * acc_exclsv_phi.read(excl_map.find(gid_minus_i)->second);
    acc_shared_tmp_phi.write(ptr, value);
  }

  while(itr_shared_tmp.has_next()) {
      const ptr_t ptr = itr_shared_tmp.next();
      acc_shared_phi.write(ptr, acc_shared_phi.read(ptr) + acc_shared_tmp_phi.read(ptr));
  }

  while(itr_excl_tmp.has_next()) {
      const ptr_t ptr = itr_excl_tmp.next();
      acc_exclsv_phi.write(ptr, acc_exclsv_phi.read(ptr) + acc_excl_tmp_phi.read(ptr));
  }
}

void
lax_calc_excl_x_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using double_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, double>;
  using size_t_acc_t = LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 3);
  assert(task->regions.size() == 3);
  assert(task->regions[0].privilege_fields.size() == 2);
  assert(task->regions[1].privilege_fields.size() == 2);
  assert(task->regions[2].privilege_fields.size() == 1);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  const double a = get_x_vel();

  double_acc_t acc_shared_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();

  double_acc_t acc_exclsv_phi = regions[1].get_field_accessor(fid_phi).typeify<double>();
  size_t_acc_t acc_exclsv_gid = regions[1].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator exclsv_itr(runtime, ctx, regions[1].get_logical_region());

  double_acc_t acc_tmp_phi = regions[2].get_field_accessor(fid_phi).typeify<double>();
  IndexIterator tmp_itr(runtime, ctx, regions[2].get_logical_region());

  std::map<size_t, ptr_t> shared_map = create_map(runtime, ctx, regions[0]);
  std::map<size_t, ptr_t> excl_map = create_map(runtime, ctx, regions[1]);

  while(exclsv_itr.has_next()) {
    const ptr_t ptr = exclsv_itr.next();
    int gid_plus_i, gid_minus_i;
    calc_x_indices(ptr, acc_exclsv_gid, &gid_plus_i, &gid_minus_i);

    double value = -a * a * acc_exclsv_phi.read(ptr);

    if (excl_map.find(gid_plus_i) != excl_map.end())
            value += 0.5 * (a * a - a) * acc_exclsv_phi.read(excl_map.find(gid_plus_i)->second);
    else if (shared_map.find(gid_plus_i) != shared_map.end())
        value += 0.5 * (a * a - a) * acc_shared_phi.read(shared_map.find(gid_plus_i)->second);

    if (excl_map.find(gid_minus_i) != excl_map.end())
        value += 0.5 * (a * a + a) * acc_exclsv_phi.read(excl_map.find(gid_minus_i)->second);
    else if (shared_map.find(gid_minus_i) != shared_map.end())
        value += 0.5 * (a * a + a) * acc_shared_phi.read(shared_map.find(gid_minus_i)->second);

    acc_tmp_phi.write(ptr, value);
  }

}

void
lax_write_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 2);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  std::ofstream myfile;
  myfile.open("lax.out");

  LegionRuntime::Accessor::RegionAccessor<generic_type, double>
     acc_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
     acc_gid = regions[0].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator itr(runtime, ctx, regions[0].get_logical_region());
    while(itr.has_next()) {
    	const ptr_t ptr = itr.next();
    	const int pt = acc_gid.read(ptr);
    	const int y_index = pt / NX;
    	const int x_index = pt % NX;
    	myfile << x_index << " " << y_index << " " << acc_phi.read(ptr) << std::endl;
    }
    myfile.close();
}

void
lax_init_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 2);

  field_ids_t & fid_t =field_ids_t::instance();
  field_id fid_phi = fid_t.fid_data;
  field_id fid_gid = fid_t.fid_cell;

  LegionRuntime::Accessor::RegionAccessor<generic_type, double>
     acc_phi = regions[0].get_field_accessor(fid_phi).typeify<double>();
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
     acc_gid = regions[0].get_field_accessor(fid_gid).typeify<size_t>();
  IndexIterator itr(runtime, ctx, regions[0].get_logical_region());
  while(itr.has_next()) {
    const ptr_t ptr = itr.next();
    const size_t pt = acc_gid.read(ptr);
    const size_t y_index = pt / NX;
    const size_t x_index = pt % NX;
    double x = static_cast<double>(x_index) / static_cast<double>(NX - 1);
    double y = static_cast<double>(y_index) / static_cast<double>(NY - 1);
    if ( (x <= 0.5) && (y <= 0.5) )
      acc_phi.write(ptr,1.0);
    else
      acc_phi.write(ptr,0.0);
  }
}

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

    LegionRuntime::Accessor::RegionAccessor<generic_type, double>
    acc_shared= regions[0].get_field_accessor(fid).typeify<double>();
    IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());
    std::set<ptr_t> shared_pts;  // TODO profile this or switch to dense storage
    while(itr_shared.has_next())
    	shared_pts.insert(itr_shared.next());

    LegionRuntime::Accessor::RegionAccessor<generic_type, double>
    acc_ghost= regions[1].get_field_accessor(fid).typeify<double>();
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

  // Initialize data
  TaskLauncher exclusive_launcher(task_ids_t::instance().lax_init_task_id, TaskArgument(nullptr, 0));
  exclusive_launcher.add_region_requirement(RegionRequirement(lr_exclusive, WRITE_DISCARD, EXCLUSIVE, lr_exclusive));
  exclusive_launcher.add_field(0, fid_phi);
  exclusive_launcher.add_field(0, fid_gid);
  Future init_exclusive_future = runtime->execute_task(ctx, exclusive_launcher);

  sprint::TaskWrapper task_wrapper(&args, lregions_ghost, pregions_ghost, lregion_halo, task_ids_t::instance().lax_halo_task_id, fid_phi);

  // phase WRITE: masters update their halo regions; slaves may not access data
  TaskLauncher shared_launcher(task_ids_t::instance().lax_init_task_id, TaskArgument(nullptr, 0));
  shared_launcher.add_region_requirement(RegionRequirement(lr_shared, WRITE_DISCARD, EXCLUSIVE, lr_shared));
  shared_launcher.add_field(0, fid_phi);
  shared_launcher.add_field(0, fid_gid);
  bool read_phase = false;
  bool write_phase = true;
  task_wrapper.execute_task(ctx, runtime, shared_launcher, read_phase, write_phase);

  const double dx = 1.0  / static_cast<double>(NX - 1);
  const double dy = 1.0  / static_cast<double>(NY - 1);
  const double dt = std::min(CFL * dx / U, CFL * dy / V);
  double time = 0.0;
  while (time < 0.33) {
    time += dt;
    for (int split = 0; split < 2; split ++) {

	  Processor::TaskFuncID split_task = task_ids_t::instance().lax_calc_excl_x_task_id;
      if (split)
        split_task = task_ids_t::instance().lax_calc_excl_y_task_id;
      TaskLauncher exclusive_launcher(split_task, TaskArgument(nullptr, 0));
      exclusive_launcher.add_region_requirement(RegionRequirement(lr_shared, READ_ONLY, EXCLUSIVE,
              lr_shared).add_field(fid_t.fid_data).add_field(fid_t.fid_cell) );
      exclusive_launcher.add_region_requirement(RegionRequirement(lr_exclusive, READ_WRITE, EXCLUSIVE,
              lr_exclusive).add_field(fid_t.fid_data).add_field(fid_t.fid_cell) );
      exclusive_launcher.add_region_requirement(RegionRequirement(lr_excl_tmp, WRITE_DISCARD, EXCLUSIVE,
              lr_excl_tmp).add_field(fid_t.fid_data) );
      runtime->execute_task(ctx, exclusive_launcher);

      // phase READ immediately followed by phase WRITE

	  split_task = task_ids_t::instance().lax_adv_x_task_id;
      if (split)
        split_task = task_ids_t::instance().lax_adv_y_task_id;
      TaskLauncher shared_launcher(split_task, TaskArgument(&my_rank, sizeof(int)));
      shared_launcher.add_region_requirement(RegionRequirement(lr_shared, READ_WRITE, EXCLUSIVE, lr_shared)
    		  .add_field(fid_phi).add_field(fid_gid));
      shared_launcher.add_region_requirement(RegionRequirement(lr_exclusive, READ_WRITE, EXCLUSIVE, lr_exclusive)
    		  .add_field(fid_phi).add_field(fid_gid));
      shared_launcher.add_region_requirement(RegionRequirement(lregion_halo, READ_ONLY, EXCLUSIVE, lregion_halo)
    		  .add_field(fid_phi));
      shared_launcher.add_region_requirement(RegionRequirement(lr_shared_tmp, WRITE_DISCARD, EXCLUSIVE, lr_shared_tmp)
    		  .add_field(fid_phi));
      shared_launcher.add_region_requirement(RegionRequirement(lr_excl_tmp, READ_ONLY, EXCLUSIVE, lr_excl_tmp)
    		  .add_field(fid_phi));
      shared_launcher.add_region_requirement(RegionRequirement(lr_ghost, READ_ONLY, EXCLUSIVE, lr_ghost)
    		  .add_field(fid_gid));
      bool read_phase = true;
      bool write_phase = true;
      task_wrapper.execute_task(ctx, runtime, shared_launcher, read_phase, write_phase);


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

