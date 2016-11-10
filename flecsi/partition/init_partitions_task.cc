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
#include "flecsi/partition/init_partitions_task.h"

namespace flecsi {
namespace dmp {

parts
get_numbers_of_cells_task(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{
  struct parts partitions; 
  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];
  
#if 0
  int rank; 
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
  
  partitions.primary = ip.primary.size();
  partitions.exclusive = ip.exclusive.size();
  partitions.shared = ip.shared.size();
  partitions.ghost = ip.ghost.size();

  std::cout << "about to return partitions (primary,exclusive,shared,ghost) ("
            << partitions.primary << "," 
            <<partitions.exclusive << "," << partitions.shared << "," 
            << partitions.ghost << ")" << std::endl;

  return partitions; 
}//get_numbers_of_cells_task

void
init_cells_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_cells" << std::endl;

  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];


  LegionRuntime::HighLevel::LogicalRegion lr = regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is = lr.get_index_space();

  LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, is);

  auto ac = regions[0].get_field_accessor(0).typeify<int>();

  for(size_t i = 0; i <ip.primary.size() ; ++i){
    assert(itr.has_next());
    size_t id = ip.primary[i];
    ptr_t ptr = itr.next();
    ac.write(ptr, id);
  }//end for

}//init_cells_task


Legion::LogicalRegion
shared_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in shared_partitioning task" << std::endl;

  using index_partition_t = index_partition__<size_t>;
  using legion_domain = LegionRuntime::HighLevel::Domain;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];

  LegionRuntime::HighLevel::LogicalRegion lr = regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is = lr.get_index_space();

  Rect<1> shared_rect(Point<1>(0), Point<1>(ip.shared.size()-1));
  LegionRuntime::HighLevel::IndexSpace shared_is =runtime->create_index_space(
          ctx, legion_domain::from_rect<1>(shared_rect));

  LegionRuntime::HighLevel::FieldSpace shared_fs =
          runtime->create_field_space(ctx);
  {
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,shared_fs);
    allocator.allocate_field(sizeof(ptr_t), FID_SHARED);
  }

  LegionRuntime::HighLevel::LogicalRegion shared_lr=
       runtime->create_logical_region(ctx,shared_is, shared_fs);
  runtime->attach_name(shared_lr, "shared temp  logical region");

  LegionRuntime::HighLevel::RegionRequirement req(shared_lr,
                      READ_WRITE, EXCLUSIVE, shared_lr);
  req.add_field(FID_SHARED);
  LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
  LegionRuntime::HighLevel::PhysicalRegion shared_region =
              runtime->map_region(ctx, shared_launcher);
  shared_region.wait_until_valid();
  LegionRuntime::Accessor::RegionAccessor<
    LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
    shared_region.get_field_accessor(FID_SHARED).typeify<ptr_t>();

  size_t indx=0;
  for(size_t i = 0; i <ip.shared.size() ; i++){
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, is);
    size_t id_s = ip.shared[i].mesh_id;
    for(size_t j = 0; j <ip.primary.size() ; j++){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      size_t id_p = ip.primary[j];
      if (id_p == id_s){
         j=ip.primary.size()+1; 
         acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(indx)),ptr);
         indx++;
      }//end if
    }//end for
  }//end for

  runtime->unmap_region(ctx, shared_region);
  return shared_lr;
}//shared_part_task

Legion::LogicalRegion
exclusive_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in exclusive_partitioning task" << std::endl;
  
  using index_partition_t = index_partition__<size_t>;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  
  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];
  
  LegionRuntime::HighLevel::LogicalRegion lr = regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is = lr.get_index_space();
  
  Rect<1> exclusive_rect(Point<1>(0), Point<1>(ip.exclusive.size()-1));
  LegionRuntime::HighLevel::IndexSpace exclusive_is =
	runtime->create_index_space(ctx, legion_domain::from_rect<1>(exclusive_rect));
  
  LegionRuntime::HighLevel::FieldSpace exclusive_fs =
          runtime->create_field_space(ctx);
  { 
    LegionRuntime::HighLevel::FieldAllocator allocator =
        runtime->create_field_allocator(ctx,exclusive_fs);
    allocator.allocate_field(sizeof(ptr_t), FID_SHARED);
  }
  
  LegionRuntime::HighLevel::LogicalRegion exclusive_lr= 
       runtime->create_logical_region(ctx,exclusive_is, exclusive_fs);
  runtime->attach_name(exclusive_lr, "exclusive temp  logical region");
  
  LegionRuntime::HighLevel::RegionRequirement req(exclusive_lr,
                      READ_WRITE, EXCLUSIVE, exclusive_lr);
  req.add_field(FID_SHARED);
  LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
  LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
              runtime->map_region(ctx, exclusive_launcher);
  exclusive_region.wait_until_valid();
  LegionRuntime::Accessor::RegionAccessor<
    LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
    exclusive_region.get_field_accessor(FID_SHARED).typeify<ptr_t>();
  
  size_t indx=0; 
  for(size_t i = 0; i <ip.exclusive.size() ; i++){
    LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, is);
    size_t id_s = ip.exclusive[i];
    for(size_t j = 0; j <ip.primary.size() ; j++){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      size_t id_p = ip.primary[j];
      if (id_p == id_s){
         j=ip.primary.size()+1;
         acc.write(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(indx)),ptr);
         indx++;
      }//end if
    }//end for
  }//end for

  runtime->unmap_region(ctx, exclusive_region);
  return exclusive_lr; 
}//exclusive_part_task

} // namespace dmp
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

