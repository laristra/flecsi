/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

//#include <flecsi/execution/legion/mapper.h>
//#include <flecsi/execution/legion/internal_task.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/internal_task.h>
///
/// \file
/// \date Initial file creation: Apr 01, 2017
///

enum FieldIDs {
  FID_VAL,
  FID_DERIV,
};

using namespace Legion;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Define a Legion task to register.
void fill_task(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context context,
    Legion::Runtime * runtime)
{
    PhysicalRegion input_region=regions[0];
    RegionAccessor<AccessorType::Generic, size_t> acc =
      input_region.get_field_accessor(FID_VAL).typeify<size_t>();
    Legion::Domain domain = runtime->get_index_space_domain(context,
            input_region.get_logical_region().get_index_space());
    LegionRuntime::Arrays::Rect<2> rect = domain.get_rect<2>();
    int count=0;
    for (LegionRuntime::Arrays::GenericPointInRectIterator<2> pir(rect); pir; pir++){
      count++;
      acc.write(DomainPoint::from_point<2>(pir.p), count);
    }
}
// Register the task. The task id is automatically generated.
flecsi_internal_register_legion_task(
    fill_task,
    processor_type_t::loc,
    index | leaf);

// Define a Legion task to register.
int check_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context context,
  Legion::Runtime * runtime) 
{
  std::cout <<"inside of the check task" <<std::endl;

  assert(regions.size() == 3);
  assert(task->regions.size() == 3);

  LogicalRegion ex_lr=task->regions[0].region;

  IndexSpace ex_is=ex_lr.get_index_space();
  
  //we need to get Rect for the parent index space in purpose to loop over
  //compacted physical instance
  IndexPartition parent_ip = runtime->get_parent_index_partition(ex_is);
  IndexSpace parent_is = runtime->get_parent_index_space(parent_ip); 

  Domain parent_dom = runtime->get_index_space_domain(context, parent_is);
  LegionRuntime::Arrays::Rect<2> parent_rect = parent_dom.get_rect<2>();

  //we get an accessor to the exclusive LR because it points to the
  //first element in the compacted physical instanse
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  RegionAccessor<AccessorType::Generic, size_t> acc =
    regions[0].get_field_accessor(fid).typeify<size_t>();

  //loop over compacted physical instanse (this will not work with
  //Legions bounds checking ON)
/*  size_t k=0;
  for (GenericPointInRectIterator<2> pir(parent_rect); pir; pir++)
  {
   k++;
#ifndef BOUNDS_CHECKS
    size_t count = acc.read(DomainPoint::from_point<1>(pir.p));
    clog_assert(count==k, "MAPPER compaction is brocken ");
    std::cout<<count<<std::endl;
#endif
  }
*/
  size_t * combined_data;

#if 0
  LegionRuntime::Arrays::Rect<2> sr;
  LegionRuntime::Accessor::ByteOffset bo[1];
  combined_data = acc.template raw_rect_ptr<2>(parent_rect, sr, bo);
#endif

  Legion::Domain ex_dom =
            runtime->get_index_space_domain(context, ex_is);
        LegionRuntime::Arrays::Rect<2> ex_rect = ex_dom.get_rect<2>();

        LegionRuntime::Arrays::Rect<2> sr;
        LegionRuntime::Accessor::ByteOffset bo[2];

        // get an accessor to the first element in exclusive LR:
        auto ac = regions[0].get_field_accessor(fid).template typeify<size_t>();
        combined_data = ac.template raw_rect_ptr<2>(ex_rect, sr, bo);

  for (size_t i=0; i<24;i++){
  std::cout <<"combined_array ["<< i<<"] = "<<*(combined_data+i)<<std::endl;
 //  clog_assert((i+1)==*(combined_data+i), " ");
  }

  return 0;
} // internal_task_example

// Register the task. The task id is automatically generated.
flecsi_internal_register_legion_task(
    check_task,
    processor_type_t::loc,
    index | leaf);
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();  
  flecsi::execution::context_t & flecsi_context =
    flecsi::execution::context_t::instance();
 
  int num_elmts = 20;
  int num_ghost = 4;
  int num_colors = flecsi_context.colors();

  LegionRuntime::Arrays::Rect<2> elem_rect =
      LegionRuntime::Arrays::Rect<2>(LegionRuntime::Arrays::Point<2>::ZEROES(),
        make_point(num_colors, num_elmts+num_ghost));
  IndexSpace is = runtime->create_index_space(context ,
                          Domain::from_rect<2>(elem_rect));
  FieldSpace fs = runtime->create_field_space(context );
  {
    FieldAllocator allocator =
      runtime->create_field_allocator(context , fs);
    allocator.allocate_field(sizeof(size_t),FID_VAL);
  }

  LogicalRegion lr = runtime->create_logical_region(context , is, fs);


  LegionRuntime::Arrays::Rect<1> color_bounds(
      LegionRuntime::Arrays::Point<1>(0),LegionRuntime::Arrays::Point<1>(
      num_colors-1));
  Domain color_domain = Domain::from_rect<1>(color_bounds);

  //creating primary partitioning
  DomainColoring primary_partitioning;
  for (int color = 0; color < num_colors; color++) {
    LegionRuntime::Arrays::Rect<2> primary_rect(
            make_point(color, 0),
            make_point(
                color, num_elmts + num_ghost - 1));
     primary_partitioning[color] = Domain::from_rect<2>(primary_rect);
  }
 
  IndexPartition primary_partition =
    runtime->create_index_partition(context , is, color_domain,
                                  primary_partitioning, true/*disjoint*/);


  LogicalPartition primary_lp =
         runtime->get_logical_partition(context, lr,
             primary_partition);


  //Fill out LR with numbers:
//  size_t key_0 = flecsi_internal_task_key(fill_task);
   const auto key_0 =
    flecsi_context.task_id<flecsi_internal_task_key(fill_task)>();

  std::cout <<"IRINA DEBUG key_0 = " <<key_0<<std::endl;
  Legion::IndexLauncher fill_launcher(key_0,
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
     Legion::TaskArgument(nullptr, 0), Legion::ArgumentMap());

  fill_launcher.add_region_requirement(
    Legion::RegionRequirement(primary_lp, 0/*projection ID*/,
      WRITE_DISCARD, EXCLUSIVE, lr))
      .add_field(FID_VAL);


  fill_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  auto fm = runtime->execute_index_space(context, fill_launcher);
  fm.wait_all_results(true);


  //creating exclusive, shared and ghost partitionings  
  IndexPartition ex_ip, sh_ip, gh_ip;
  {
    DomainColoring ex_coloring, sh_coloring, gh_coloring;
    int index = 0;
    // Iterate over all the colors and compute the entry
    // for both partitions for each color.
    for (int color = 0; color < num_colors; color++)
    {
      LegionRuntime::Arrays::Rect<2> subrect1(
        make_point(color, 0),
            make_point(color, num_elmts - num_ghost - 1));
      LegionRuntime::Arrays::Rect<2> subrect2(
        make_point(color, num_elmts - num_ghost - 1),
            make_point(color, num_elmts  - 1));
      LegionRuntime::Arrays::Rect<2> subrect3(
        make_point(color, num_elmts  - 1),
            make_point(color, num_elmts + num_ghost - 1));

      ex_coloring[color] = Domain::from_rect<2>(subrect1);
      sh_coloring[color] = Domain::from_rect<2>(subrect2);
      gh_coloring[color] = Domain::from_rect<2>(subrect3);
    }
    ex_ip = runtime->create_index_partition(context , is, color_domain,
                                    ex_coloring, true/*disjoint*/);
    sh_ip = runtime->create_index_partition(context , is, color_domain,
                                    sh_coloring, true/*disjoint*/);
    gh_ip = runtime->create_index_partition(context , is, color_domain,
                                    gh_coloring, true/*disjoint*/);
  }

  LogicalPartition ex_lp =
    runtime->get_logical_partition(context, lr, ex_ip);
  LogicalPartition sh_lp =
    runtime->get_logical_partition(context, lr, sh_ip);
  LogicalPartition gh_lp =
    runtime->get_logical_partition(context, lr, gh_ip);
 
  auto key_1 = flecsi_context.task_id<flecsi_internal_task_key(check_task)>();

  Legion::IndexLauncher check_launcher(key_1, 
      Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
     Legion::TaskArgument(nullptr, 0), Legion::ArgumentMap());

  Legion::MappingTagID tag = EXCLUSIVE_LR; 
  check_launcher.add_region_requirement(
      RegionRequirement(ex_lp, 0/*projection ID*/,
                       READ_ONLY, EXCLUSIVE, lr, tag))
      .add_field(FID_VAL);
//  check_launcher.add_region_requirement(
//      RegionRequirement(sh_lp, 0/*projection ID*/,
//                        READ_ONLY, EXCLUSIVE, lr))
//      .add_field(FID_VAL);
  check_launcher.add_region_requirement(
      RegionRequirement(gh_lp, 0/*projection ID*/,
                        READ_ONLY, EXCLUSIVE, lr))
      .add_field(FID_VAL);

  check_launcher.add_region_requirement(
      RegionRequirement(sh_lp, 0/*projection ID*/,
                        READ_ONLY, EXCLUSIVE, lr))
      .add_field(FID_VAL);

  check_launcher.tag=MAPPER_COMPACTED_STORAGE;
  check_launcher.tag = MAPPER_FORCE_RANK_MATCH;
 auto fm2 = runtime->execute_index_space(context, check_launcher);
 fm2.wait_all_results();
} // driver


} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
