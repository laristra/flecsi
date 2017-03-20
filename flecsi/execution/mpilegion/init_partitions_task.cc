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

#include <iostream>

#include "flecsi/execution/context.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/execution/mpilegion/init_partitions_task.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;

namespace flecsi {
namespace execution {
namespace sprint {

parts
get_numbers_of_cells_task(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{
  struct parts partitions; 
  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  //getting cells partitioning info from MPI
  index_partition_t ip_cells =
    context_.interop_helper_.data_storage_[0];
  
  //getting vertices partitioning info from MPI
  index_partition_t ip_vertices =
    context_.interop_helper_.data_storage_[1];

  std::vector<std::pair<size_t, size_t>> raw_cell_vertex_conns =
      context_.interop_helper_.data_storage_[2];  
#if 0
  size_t rank; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in get_numbers_of_cells native legion task rank is " <<
       rank <<  std::endl;

  for(size_t i=0; i<ip.primary.size(); i++) {
    auto element = ip.primary[i];
    std::cout << " Found ghost elemment: " << element <<
        " on rank: " << rank << std::endl;
  }
    
  for(size_t i=0; i< ip.exclusive.size();i++ ) {
    auto element = ip.exclusive[i];
    std::cout << " Found exclusive elemment: " << element << 
        " on rank: " << rank << std::endl;
  }
  
  for(size_t i=0; i< ip.shared.size(); i++) {
    auto element = ip.shared_id(i);
    std::cout << " Found shared elemment: " << element << 
        " on rank: " << rank << std::endl;
  } 

  for(size_t i=0; i<ip.ghost.size(); i++) {
    auto element = ip.ghost_id(i);
    std::cout << " Found ghost elemment: " << element << 
        " on rank: " << rank << std::endl;
  }

#endif
  
  partitions.primary_cells = ip_cells.primary.size();
  partitions.exclusive_cells = ip_cells.exclusive.size();
  partitions.shared_cells = ip_cells.shared.size();
  partitions.ghost_cells = ip_cells.ghost.size();

  partitions.primary_vertices = ip_vertices.primary.size();
  partitions.exclusive_vertices = ip_vertices.exclusive.size();
  partitions.shared_vertices = ip_vertices.shared.size();
  partitions.ghost_vertices = ip_vertices.ghost.size();

  partitions.vertex_conns = raw_cell_vertex_conns.size();

  std::cout << "about to return partitions (primary,exclusive,shared,ghost) ("
            << partitions.primary_cells << "," 
            <<partitions.exclusive_cells << "," << partitions.shared_cells <<
             "," << partitions.ghost_cells << ")" << std::endl;

  return partitions; 
}//get_numbers_of_cells_task

void
initialization_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{

  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  std::cout << "Here I am in init_cells" << std::endl;

  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using field_id = LegionRuntime::HighLevel::FieldID;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip_cells =
    context_.interop_helper_.data_storage_[0];
  index_partition_t ip_vert =
    context_.interop_helper_.data_storage_[1];

  //cells:
  LegionRuntime::HighLevel::LogicalRegion lr_cells =
      regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is_cells = lr_cells.get_index_space();

  LegionRuntime::HighLevel::IndexIterator itr_cells(runtime, ctx, is_cells);

//  auto acc_cells = regions[0].get_field_accessor(0).typeify<size_t>();

  field_id fid_cell = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>  acc_cells = regions[0].get_field_accessor(fid_cell).typeify<size_t>();


  for (auto primary_cell : ip_cells.primary) {
    assert(itr_cells.has_next());
    size_t id =primary_cell;
    ptr_t ptr = itr_cells.next();
    acc_cells.write(ptr, id);
  }

  //vertices
  LegionRuntime::HighLevel::LogicalRegion lr_vert =
      regions[1].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is_vert = lr_vert.get_index_space();

  LegionRuntime::HighLevel::IndexIterator itr_vert(runtime, ctx, is_vert);

  //auto acc_vert = regions[1].get_field_accessor(0).typeify<size_t>();

  field_id fid_vert = *(task->regions[1].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, size_t>  acc_vert = regions[1].get_field_accessor(fid_vert).typeify<size_t>();



  for (auto primary_vert : ip_vert.primary) {
    assert(itr_vert.has_next());
    size_t id =primary_vert;
    ptr_t ptr = itr_vert.next();
    acc_vert.write(ptr, id);
  }

}//initialization_task


partition_lr
shared_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);

  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  field_ids_t & fid_t =field_ids_t::instance();

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip_cells =
    context_.interop_helper_.data_storage_[0];
  index_partition_t ip_vert =
    context_.interop_helper_.data_storage_[1];

  partition_lr shared_lr;

  LegionRuntime::HighLevel::LogicalRegion cells_lr =
    regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace cells_is = cells_lr.get_index_space();

  LegionRuntime::Arrays::Rect<1> cells_shared_rect(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(ip_cells.shared.size()-1));
  LegionRuntime::HighLevel::IndexSpace cells_shared_is =
          runtime->create_index_space(
          ctx, legion_domain::from_rect<1>(cells_shared_rect));

  LegionRuntime::HighLevel::FieldSpace cells_shared_fs =
          runtime->create_field_space(ctx);
  {
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,cells_shared_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }

  shared_lr.cells=
       runtime->create_logical_region(ctx,cells_shared_is, cells_shared_fs);
  runtime->attach_name(shared_lr.cells, "shared temp  logical region");

  {
    LegionRuntime::HighLevel::RegionRequirement req(shared_lr.cells,
                      READ_WRITE, EXCLUSIVE, shared_lr.cells);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion shared_region =
              runtime->map_region(ctx, shared_launcher);
    shared_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
      shared_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    int indx=0;
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, cells_is);
    ptr_t start=itr.next();
    for (auto shared_cell : ip_cells.shared) {
      ptr_t ptr = (start.value+shared_cell.offset);
      acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(indx)),ptr);
      indx++;
    }//end for

    runtime->unmap_region(ctx, shared_region);
  }//scope


  LegionRuntime::HighLevel::LogicalRegion vert_lr =
    regions[1].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace vert_is = vert_lr.get_index_space();

  LegionRuntime::Arrays::Rect<1> vert_shared_rect(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(ip_vert.shared.size()-1));
  LegionRuntime::HighLevel::IndexSpace vert_shared_is =
          runtime->create_index_space(
          ctx, legion_domain::from_rect<1>(vert_shared_rect));

  LegionRuntime::HighLevel::FieldSpace vert_shared_fs =
          runtime->create_field_space(ctx);
  {
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,vert_shared_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }

  shared_lr.vert=
       runtime->create_logical_region(ctx,vert_shared_is, vert_shared_fs);
  runtime->attach_name(shared_lr.vert, "shared temp  logical region");

  {
    LegionRuntime::HighLevel::RegionRequirement req(shared_lr.vert,
                      READ_WRITE, EXCLUSIVE, shared_lr.vert);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion shared_region =
              runtime->map_region(ctx, shared_launcher);
    shared_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
      shared_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    size_t indx=0;
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, vert_is);
    ptr_t start=itr.next();

    for (auto shared_vert : ip_vert.shared) {
      ptr_t ptr = (start.value+shared_vert.offset);
      acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(indx)),ptr);
      indx++;
    }//end for

    runtime->unmap_region(ctx, shared_region);
  }//scope
 

  return shared_lr;
}//shared_part_task

partition_lr
exclusive_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  
  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  field_ids_t & fid_t =field_ids_t::instance();
  
  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip_cells =
    context_.interop_helper_.data_storage_[0];
  index_partition_t ip_vert =
    context_.interop_helper_.data_storage_[1];
 
  partition_lr exclusive_lr;
 
  LegionRuntime::HighLevel::LogicalRegion cells_lr =
      regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace cells_is = cells_lr.get_index_space();
  LegionRuntime::HighLevel::LogicalRegion vert_lr =
      regions[1].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace vert_is = vert_lr.get_index_space();
  
  LegionRuntime::Arrays::Rect<1> cells_exclusive_rect(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(
      ip_cells.exclusive.size()-1));
  LegionRuntime::HighLevel::IndexSpace cells_exclusive_is =
	runtime->create_index_space(ctx, legion_domain::from_rect<1>(
      cells_exclusive_rect));
  
  LegionRuntime::HighLevel::FieldSpace cells_exclusive_fs =
          runtime->create_field_space(ctx);
  { 
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,cells_exclusive_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }
  
  exclusive_lr.cells=runtime->create_logical_region(ctx,cells_exclusive_is, 
       cells_exclusive_fs);
  runtime->attach_name(exclusive_lr.cells, 
          "exclusive temp  logical region");
  {
    LegionRuntime::HighLevel::RegionRequirement req(exclusive_lr.cells,
                      READ_WRITE, EXCLUSIVE, exclusive_lr.cells);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
              runtime->map_region(ctx, exclusive_launcher);
    exclusive_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
      exclusive_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    size_t indx=0;
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, cells_is);
    ptr_t start=itr.next();
    for (auto exclusive_cell : ip_cells.exclusive) {
      ptr_t ptr = (start.value+exclusive_cell.offset);
      acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(indx)),ptr);
      indx++;
    }//end for

    runtime->unmap_region(ctx, exclusive_region);
  }//end scope


  LegionRuntime::Arrays::Rect<1> vert_exclusive_rect(
    LegionRuntime::Arrays::Point<1>(0), LegionRuntime::Arrays::Point<1>(
      ip_vert.exclusive.size()-1));
  LegionRuntime::HighLevel::IndexSpace vert_exclusive_is =
  runtime->create_index_space(ctx, legion_domain::from_rect<1>(
      vert_exclusive_rect));

  LegionRuntime::HighLevel::FieldSpace vert_exclusive_fs =
          runtime->create_field_space(ctx);
  {
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx, vert_exclusive_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }

  exclusive_lr.vert=runtime->create_logical_region(ctx, vert_exclusive_is,
       vert_exclusive_fs);
  runtime->attach_name(exclusive_lr.vert,
          "exclusive temp  logical region");
  {
    LegionRuntime::HighLevel::RegionRequirement req(exclusive_lr.vert,
                      READ_WRITE, EXCLUSIVE, exclusive_lr.vert);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
              runtime->map_region(ctx, exclusive_launcher);
    exclusive_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
      exclusive_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    size_t indx=0;
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, vert_is);
    ptr_t start=itr.next();
    for (auto exclusive_vert : ip_vert.exclusive) {
      ptr_t ptr = (start.value+exclusive_vert.offset);
      acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(indx)),ptr);
      indx++;
    }//end for

    runtime->unmap_region(ctx, exclusive_region);
  }//end scope


  return exclusive_lr; 
}//exclusive_part_task

partition_lr
ghost_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  
  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic; 
  using field_id = LegionRuntime::HighLevel::FieldID;
  field_ids_t & fid_t =field_ids_t::instance(); 

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip_cells =
    context_.interop_helper_.data_storage_[0];
  index_partition_t ip_vert =
    context_.interop_helper_.data_storage_[1];

  partition_lr ghost_lr;

  field_id fid_cells_global = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::HighLevel::LogicalRegion cells_lr =
      regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace cells_is = cells_lr.get_index_space();
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_cells_global= regions[0].get_field_accessor(
      fid_cells_global).typeify<size_t>();

  field_id fid_vert_global = *(task->regions[1].privilege_fields.begin());
  LegionRuntime::HighLevel::LogicalRegion vert_lr =
      regions[1].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace vert_is = vert_lr.get_index_space();
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_vert_global= regions[1].get_field_accessor(
      fid_vert_global).typeify<size_t>();
 
  LegionRuntime::Arrays::Rect<1> ghost_cells_rect(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(ip_cells.ghost.size()-1));
  LegionRuntime::HighLevel::IndexSpace ghost_cells_is =
  runtime->create_index_space(
    ctx, legion_domain::from_rect<1>(ghost_cells_rect));

  LegionRuntime::HighLevel::FieldSpace ghost_cells_fs =
          runtime->create_field_space(ctx);
  { 
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,ghost_cells_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }
  
  ghost_lr.cells= runtime->create_logical_region(ctx,ghost_cells_is,
        ghost_cells_fs);
  runtime->attach_name(ghost_lr.cells, "ghost temp  logical region");
 
  {
    LegionRuntime::HighLevel::RegionRequirement req(ghost_lr.cells,
                        READ_WRITE, EXCLUSIVE, ghost_lr.cells);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher ghost_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion ghost_region =
                runtime->map_region(ctx, ghost_launcher);
    ghost_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<generic_type, ptr_t> acc =
      ghost_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    size_t indx=0;
    for (auto ghost_cell : ip_cells.ghost) {
      LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, cells_is);
      size_t id_ghost = ghost_cell.id;
      while(itr.has_next()){
        ptr_t ptr = itr.next();
        size_t id_global = acc_cells_global.read(ptr);
        if (id_global == id_ghost){
           acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
              LegionRuntime::Arrays::make_point(indx)),ptr);
           indx++;
        }//end while
      }//end for
    }//end for

    runtime->unmap_region(ctx, ghost_region);
  }//end scope

  LegionRuntime::Arrays::Rect<1> ghost_vert_rect(
    LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(ip_vert.ghost.size()-1));
  LegionRuntime::HighLevel::IndexSpace ghost_vert_is =
  runtime->create_index_space(
    ctx, legion_domain::from_rect<1>(ghost_vert_rect));

  LegionRuntime::HighLevel::FieldSpace ghost_vert_fs =
          runtime->create_field_space(ctx);
  {
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,ghost_vert_fs);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }

  ghost_lr.vert= runtime->create_logical_region(ctx,ghost_vert_is,
        ghost_vert_fs);
  runtime->attach_name(ghost_lr.vert, "ghost temp  logical region");

  {
    LegionRuntime::HighLevel::RegionRequirement req(ghost_lr.vert,
                        READ_WRITE, EXCLUSIVE, ghost_lr.vert);
    req.add_field(fid_t.fid_ptr_t);
    LegionRuntime::HighLevel::InlineLauncher ghost_launcher(req);
    LegionRuntime::HighLevel::PhysicalRegion ghost_region =
                runtime->map_region(ctx, ghost_launcher);
    ghost_region.wait_until_valid();
    LegionRuntime::Accessor::RegionAccessor<generic_type, ptr_t> acc =
      ghost_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    size_t indx=0;
    for (auto ghost_cell : ip_vert.ghost) {
      LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, vert_is);
      size_t id_ghost = ghost_cell.id;
      while(itr.has_next()){
        ptr_t ptr = itr.next();
        size_t id_global = acc_vert_global.read(ptr);
        if (id_global == id_ghost){
           acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
              LegionRuntime::Arrays::make_point(indx)),ptr);
           indx++;
        }//end while
      }//end for
    }//end for

    runtime->unmap_region(ctx, ghost_region);
  }//end scope

  return ghost_lr;
}//ghost_part_task

void
first_compaction_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 4);
  assert(task->regions.size() == 4);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  assert(task->regions[2].privilege_fields.size() == 1);
  assert(task->regions[3].privilege_fields.size() == 1);


  using field_id = LegionRuntime::HighLevel::FieldID;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  const int my_color = task->index_point.point_data[0];

  field_id fid_legion_shared = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_legion_shared = regions[0].get_field_accessor(fid_legion_shared).typeify<size_t>();
  LegionRuntime::HighLevel::IndexIterator itr_legion_shared(runtime, ctx, regions[0].get_logical_region().get_index_space());

  field_id fid_legion_exclusive = *(task->regions[1].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_legion_exclusive = regions[1].get_field_accessor(fid_legion_exclusive).typeify<size_t>();
  LegionRuntime::HighLevel::IndexIterator itr_legion_exclusive(runtime, ctx, regions[1].get_logical_region().get_index_space());

  field_id fid_flecsi_exclusive = *(task->regions[2].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_flecsi_exclusive = regions[2].get_field_accessor(fid_flecsi_exclusive).typeify<size_t>();
  Domain flecsi_exclusive_dom = runtime->get_index_space_domain(ctx, regions[2].get_logical_region().get_index_space());
  Rect<1> flecsi_exclusive_rect = flecsi_exclusive_dom.get_rect<1>();

  field_id fid_flecsi_shared = *(task->regions[3].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_flecsi_shared = regions[3].get_field_accessor(fid_flecsi_shared).typeify<size_t>();
  Domain flecsi_shared_dom = runtime->get_index_space_domain(ctx, regions[3].get_logical_region().get_index_space());
  Rect<1> flecsi_shared_rect = flecsi_shared_dom.get_rect<1>();

  for (GenericPointInRectIterator<1> flecsi_pir(flecsi_shared_rect); flecsi_pir; flecsi_pir++) {
    ptr_t legion_ptr = itr_legion_shared.next();
    acc_flecsi_shared.write(DomainPoint::from_point<1>(flecsi_pir.p), acc_legion_shared.read(legion_ptr));
  }

}

void
check_partitioning_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 6);
  assert(task->regions.size() == 6);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  assert(task->regions[2].privilege_fields.size() == 1);
  assert(task->regions[3].privilege_fields.size() == 1);
  assert(task->regions[4].privilege_fields.size() == 1);
  assert(task->regions[5].privilege_fields.size() == 1);

  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  const int my_color = task->index_point.point_data[0];
  //checking cells:
  {
    LegionRuntime::HighLevel::LogicalRegion lr_shared =
       regions[0].get_logical_region();
   LegionRuntime::HighLevel::IndexSpace is_shared = lr_shared.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_shared(runtime, ctx, is_shared);
    field_id fid_shared = *(task->regions[0].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_shared= regions[0].get_field_accessor(fid_shared).typeify<size_t>();

    LegionRuntime::HighLevel::LogicalRegion lr_exclusive = 
        regions[1].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_exclusive =
        lr_exclusive.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_exclusive(runtime,
        ctx, is_exclusive);
    field_id fid_exclusive = *(task->regions[1].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_exclusive=regions[1].get_field_accessor(
          fid_exclusive).typeify<size_t>();


    LegionRuntime::HighLevel::LogicalRegion lr_ghost = 
        regions[2].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_ghost = lr_ghost.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_ghost(runtime, ctx, is_ghost);
    field_id fid_ghost = *(task->regions[2].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_ghost= regions[2].get_field_accessor(fid_ghost).typeify<size_t>();  

    flecsi::execution::context_t & context_ =
      flecsi::execution::context_t::instance();
    index_partition_t ip =
      context_.interop_helper_.data_storage_[0];

    size_t indx = 0;
    for (auto shared_cell : ip.shared) {
    assert(itr_shared.has_next());
     ptr_t ptr=itr_shared.next();
     assert(shared_cell.id == acc_shared.read(ptr));
     std::cout << my_color << " shared " << indx << " = " << shared_cell.id << std::endl;
     indx++;
    }
    assert (indx == ip.shared.size());

    indx = 0;
    for (auto exclusive_cell : ip.exclusive) {
     assert(itr_exclusive.has_next());
     ptr_t ptr=itr_exclusive.next();
     assert(exclusive_cell.id == acc_exclusive.read(ptr));
     std::cout << my_color << " exclusive " << indx << " = " << exclusive_cell.id << std::endl;
    indx++;
    }
    assert (indx == ip.exclusive.size());


    indx = 0;
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      bool found=false;
      size_t ghost_id = acc_ghost.read(ptr);
      for (auto ghost_cell : ip.ghost) {
        if (ghost_cell.id == ghost_id){
          found=true;
        }
      }
      assert(found);
      indx++;
     }
     assert (indx == ip.ghost.size());
    }//scope

    //checking vertices
    {
     LegionRuntime::HighLevel::LogicalRegion lr_shared =
       regions[3].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_shared=lr_shared.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_shared(runtime, ctx, is_shared);
    field_id fid_shared = *(task->regions[3].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_shared= regions[3].get_field_accessor(fid_shared).typeify<size_t>();

    LegionRuntime::HighLevel::LogicalRegion lr_exclusive =
        regions[4].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_exclusive =
        lr_exclusive.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_exclusive(runtime,
        ctx, is_exclusive);
    field_id fid_exclusive = *(task->regions[4].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_exclusive=regions[4].get_field_accessor(
      fid_exclusive).typeify<size_t>();


    LegionRuntime::HighLevel::LogicalRegion lr_ghost =
        regions[5].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_ghost = lr_ghost.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_ghost(runtime, ctx, is_ghost);
    field_id fid_ghost = *(task->regions[5].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_ghost= regions[5].get_field_accessor(fid_ghost).typeify<size_t>();

    flecsi::execution::context_t & context_ =
      flecsi::execution::context_t::instance();
    index_partition_t ip =
      context_.interop_helper_.data_storage_[1];

    size_t indx = 0;
    for (auto shared_cell : ip.shared) {
    assert(itr_shared.has_next());
     ptr_t ptr=itr_shared.next();
     assert(shared_cell.id == acc_shared.read(ptr));
     indx++;
    }
    assert (indx == ip.shared.size());

    indx = 0;
    for (auto exclusive_cell : ip.exclusive) {
     assert(itr_exclusive.has_next());
     ptr_t ptr=itr_exclusive.next();
     assert(exclusive_cell.id == acc_exclusive.read(ptr));
    indx++;
    }
    assert (indx == ip.exclusive.size());
    
    indx = 0;
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      bool found=false;
      size_t ghost_id = acc_ghost.read(ptr);
      for (auto ghost_cell : ip.ghost) {
        if (ghost_cell.id == ghost_id){
          found=true;
        }
      }
      assert(found);
      indx++;
     }
     assert (indx == ip.ghost.size());

    }

  std::cout << "test for shared/ghost/exclusive partitions ... passed" 
  << std::endl;
}//check_partitioning_task

void
init_raw_conn_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();

  field_ids_t & fid_t = field_ids_t::instance();

  std::vector<std::pair<size_t, size_t>> raw_cell_vertex_conns =
      context_.interop_helper_.data_storage_[2];

  LogicalRegion raw_conn_lr = regions[0].get_logical_region();
  IndexSpace raw_conn_is = raw_conn_lr.get_index_space();

  auto raw_conn_ac = 
    regions[0].get_field_accessor(fid_t.fid_entity_pair).
    typeify<std::pair<size_t, size_t>>();

  size_t i = 0;
  IndexIterator raw_conn_itr(runtime, ctx, raw_conn_is);
  while(raw_conn_itr.has_next()){
    ptr_t ptr = raw_conn_itr.next();
    raw_conn_ac.write(ptr, raw_cell_vertex_conns[i++]);
  }
}

template<class Type>
static size_t archiveScalar(Type scalar, void* bit_stream)
{
    memcpy(bit_stream, (void*)(&scalar), sizeof(Type));
    return sizeof(Type);
}

template<class Type>
static size_t archiveVector(std::vector<Type> vec, void* bit_stream)
{
    unsigned char *serialized = (unsigned char*)(bit_stream) ;

    size_t size_size = archiveScalar(vec.size(), (void*)serialized);
    serialized += size_size;

    size_t vec_size = vec.size() * sizeof(Type);
    memcpy((void*)serialized, (void*)vec.data(), vec_size);

    return size_size + vec_size;
}

void SPMDArgsSerializer::archive(SPMDArgs* spmd_args)
{
    assert(spmd_args != nullptr);

    bit_stream_size = sizeof(PhaseBarrier) + sizeof(size_t)
            + spmd_args->masters_pbarriers.size() * sizeof(PhaseBarrier);
    bit_stream = malloc(bit_stream_size);
    free_bit_stream = true;

    unsigned char *serialized = (unsigned char*)(bit_stream);

    size_t stream_size = 0;
    stream_size += archiveScalar(spmd_args->pbarrier_as_master, (void*)(serialized+stream_size));
    stream_size += archiveVector(spmd_args->masters_pbarriers, (void*)(serialized+stream_size));

    assert(stream_size == bit_stream_size);
}

template<class Type>
static size_t restoreScalar(Type* scalar, void* bit_stream)
{
    memcpy((void*)scalar, bit_stream, sizeof(Type));
    return sizeof(Type);
}

template<class Type>
static size_t restoreVector(std::vector<Type>* vec, void* bit_stream)
{
    unsigned char *serialized = (unsigned char*)(bit_stream) ;

    size_t n_entries;
    size_t size_size = restoreScalar(&n_entries, (void*)serialized);
    serialized += size_size;

    vec->resize(n_entries);
    size_t vec_size = n_entries * sizeof(Type);
    memcpy((void*)vec->data(), (void*)serialized, vec_size);

    return size_size + vec_size;
}

void SPMDArgsSerializer::restore(SPMDArgs* spmd_args)
{
    assert(spmd_args != nullptr);
    assert(bit_stream != nullptr);

    unsigned char *serialized_args = (unsigned char *) bit_stream;

    bit_stream_size = 0;
    bit_stream_size += restoreScalar(&(spmd_args->pbarrier_as_master), (void*)(serialized_args + bit_stream_size));
    bit_stream_size += restoreVector(&(spmd_args->masters_pbarriers), (void*)(serialized_args + bit_stream_size));
}

void* ArgsSerializer::getBitStream()
{
    return bit_stream;
}

size_t ArgsSerializer::getBitStreamSize()
{
    return bit_stream_size;
}

void ArgsSerializer::setBitStream(void* stream)
{
    bit_stream = stream;
};

void
halo_copy_task(
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
ghost_init_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
    using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
    using field_id = LegionRuntime::HighLevel::FieldID;

    assert(regions.size() == 1);
    assert(task->regions.size() == 1);
    assert(task->regions[0].privilege_fields.size() == 1);
    assert(task->arglen == sizeof(int));
    int cycle = *(const int*)task->args;
    field_id fid = *(task->regions[0].privilege_fields.begin());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_shared= regions[0].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());
    while(itr_shared.has_next()) {
        ptr_t ptr = itr_shared.next();
        acc_shared.write(ptr, static_cast<size_t>(ptr.value + cycle));
    }
}

void
ghost_check_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
    using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
    using field_id = LegionRuntime::HighLevel::FieldID;

    assert(regions.size() == 1);
    assert(task->regions.size() == 1);
    assert(task->regions[0].privilege_fields.size() == 1);
    assert(task->arglen == sizeof(int));
    int cycle = *(const int*)task->args;
    field_id fid = *(task->regions[0].privilege_fields.begin());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_halo= regions[0].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_halo(runtime, ctx, regions[0].get_logical_region());
    while(itr_halo.has_next()) {
        ptr_t ptr = itr_halo.next();
        assert(acc_halo.read(ptr) == static_cast<size_t>(ptr.value + cycle));
    }
}

TaskWrapper::TaskWrapper(
		SPMDArgs* args,
		std::vector<LogicalRegion> lrs_shared,
		std::vector<PhysicalRegion> prs_shared,
		LogicalRegion lr_ghost,
		size_t copy_task_id,
		FieldID fid) :
	spmd_args(args),
	lregions_neighbors_shared(lrs_shared),
	pregions_neighbors_shared(prs_shared),
	lregion_ghost(lr_ghost),
	ghost_copy_task_id(copy_task_id),
	ghost_fid(fid),
	is_readable(true) {
}

void TaskWrapper::write_prologue(TaskLauncher& task_launcher) {
	// as master
	task_launcher.add_wait_barrier(spmd_args->pbarrier_as_master);                     // phase WRITE
	task_launcher.add_arrival_barrier(spmd_args->pbarrier_as_master);                  // phase READ
}

void TaskWrapper::write_epilogue(Legion::Context ctx, Legion::HighLevelRuntime *runtime) {
	// as master
	spmd_args->pbarrier_as_master = runtime->advance_phase_barrier(ctx, spmd_args->pbarrier_as_master);   // phase READ

	// as slave
    for (int master=0; master < spmd_args->masters_pbarriers.size(); master++) {
    	spmd_args->masters_pbarriers[master].arrive(1);                                     // phase READ
    	spmd_args->masters_pbarriers[master] =
                runtime->advance_phase_barrier(ctx, spmd_args->masters_pbarriers[master]);  // phase READ
    }

    is_readable = false;
}

void TaskWrapper::read_prologue(Legion::Context ctx, Legion::HighLevelRuntime *runtime) {

	if (!is_readable) {
		// as master
		spmd_args->pbarrier_as_master.arrive(1);                                                // phase WRITE
		spmd_args->pbarrier_as_master =
            runtime->advance_phase_barrier(ctx, spmd_args->pbarrier_as_master);             	// phase WRITE

		// as slave

		for (int master=0; master < spmd_args->masters_pbarriers.size(); master++) {
			AcquireLauncher acquire_launcher(lregions_neighbors_shared[master], lregions_neighbors_shared[master],
        		  pregions_neighbors_shared[master]);
			acquire_launcher.add_field(ghost_fid);
			acquire_launcher.add_wait_barrier(spmd_args->masters_pbarriers[master]);            // phase READ
			runtime->issue_acquire(ctx, acquire_launcher);

			TaskLauncher launcher(ghost_copy_task_id, TaskArgument(nullptr, 0));
			launcher.add_region_requirement(RegionRequirement(lregions_neighbors_shared[master], READ_ONLY, EXCLUSIVE,
    			  lregions_neighbors_shared[master]));
			launcher.add_field(0, ghost_fid);
			launcher.add_region_requirement(RegionRequirement(lregion_ghost, READ_WRITE, EXCLUSIVE, lregion_ghost));
			launcher.add_field(1, ghost_fid);
			runtime->execute_task(ctx, launcher);

			ReleaseLauncher release_launcher(lregions_neighbors_shared[master], lregions_neighbors_shared[master],
				pregions_neighbors_shared[master]);
			release_launcher.add_field(ghost_fid);
			release_launcher.add_arrival_barrier(spmd_args->masters_pbarriers[master]);     // phase WRITE
			runtime->issue_release(ctx, release_launcher);

			spmd_args->masters_pbarriers[master] =
          		runtime->advance_phase_barrier(ctx, spmd_args->masters_pbarriers[master]);  // phase WRITE
		} // for master as slave

		is_readable = true;
	} // if !is_readable
}

Future TaskWrapper::execute_task(Legion::Context ctx, Legion::HighLevelRuntime *runtime,
		TaskLauncher& task_launcher, bool read_phase, bool write_phase) {

	if (read_phase)
		read_prologue(ctx, runtime);

	if (write_phase)
		write_prologue(task_launcher);

	Future value = runtime->execute_task(ctx, task_launcher);

	if (write_phase)
		write_epilogue(ctx, runtime);

	return value;
}

void
ghost_access_task(
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

  SPMDArgs args;
  SPMDArgsSerializer args_serializer;
  args_serializer.setBitStream(task->args);
  args_serializer.restore(&args);

  LogicalRegion lr_shared = regions[0].get_logical_region();
  runtime->unmap_region(ctx, regions[0]);

  //LegionRuntime::HighLevel::LogicalRegion lr_exclusive = regions[1].get_logical_region();

  std::vector<LogicalRegion> lregions_ghost;
  std::vector<PhysicalRegion> pregions_ghost;
  for (int index = 2; index < regions.size(); index++) {
	  LogicalRegion lr_ghost = regions[index].get_logical_region();
	  lregions_ghost.push_back(lr_ghost);
	  pregions_ghost.push_back(regions[index]);
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

  TaskWrapper task_wrapper(&args, lregions_ghost, pregions_ghost, lregion_halo, task_ids_t::instance().halo_copy_task_id, fid);

  for (int cycle = 0; cycle < 2; cycle++) {

    // phase WRITE: masters update their halo regions; slaves may not access data

    {
      TaskLauncher shared_launcher(task_ids_t::instance().ghost_init_task_id, TaskArgument(&cycle, sizeof(int)));
      shared_launcher.add_region_requirement(RegionRequirement(lr_shared, READ_WRITE, EXCLUSIVE, lr_shared));
      shared_launcher.add_field(0, fid);
      bool read_phase = false;
      bool write_phase = true;
      task_wrapper.execute_task(ctx, runtime, shared_launcher, read_phase, write_phase);
      std::cout << my_rank << " as master writes data; phase 1 of cycle " << cycle << std::endl;
    }

    // phase READ: slaves can read data; masters may not write to data

    {
      TaskLauncher check_launcher(task_ids_t::instance().ghost_check_task_id, TaskArgument(&cycle, sizeof(int)));
      check_launcher.add_region_requirement(RegionRequirement(lregion_halo, READ_ONLY, EXCLUSIVE, lregion_halo));
      check_launcher.add_field(0, fid);
      bool read_phase = true;
      bool write_phase = false;
      task_wrapper.execute_task(ctx, runtime, check_launcher, read_phase, write_phase);
    }
  } // cycle


  std::cout << "test ghost access ... passed"
  << std::endl;
}//ghost_access_task

} // namespace sprint
} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

