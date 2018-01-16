/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <flecsi/execution/legion/mapper.h>
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
    RegionAccessor<AccessorType::Generic, double> acc =
      input_region.get_field_accessor(FID_VAL).typeify<double>();
    Legion::Domain domain = runtime->get_index_space_domain(context,
            input_region.get_logical_region().get_index_space());
    LegionRuntime::Arrays::Rect<1> rect = domain.get_rect<1>();
    int count=0;
    for (LegionRuntime::Arrays::GenericPointInRectIterator<1> pir(rect); pir; pir++){
      count++;
      acc.write(DomainPoint::from_point<1>(pir.p), count);
    }
}
// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(fill_task,
  processor_type_t::loc, single|leaf);

// Define a Legion task to register.
int internal_task_example_1(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context context,
  Legion::Runtime * runtime) 
{
  std::cout <<"inside of the task1" <<std::endl;

  assert(regions.size() == 3);
  assert(task->regions.size() == 3);

  LogicalRegion ex_lr=task->regions[0].region;

  IndexSpace ex_is=ex_lr.get_index_space();
  
  //we need to get Rect for the parent index space in purpose to loop over
  //compacted physical instance
  IndexPartition parent_ip = runtime->get_parent_index_partition(ex_is);
  IndexSpace parent_is = runtime->get_parent_index_space(parent_ip); 

  Domain parent_dom = runtime->get_index_space_domain(context, parent_is);
  LegionRuntime::Arrays::Rect<1> parent_rect = parent_dom.get_rect<1>();

  //we get an accessor to the exclusive LR because it points to the
  //first element in the compacted physical instanse
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  RegionAccessor<AccessorType::Generic, double> acc =
    regions[0].get_field_accessor(fid).typeify<double>();

  //loop over compacted physical instanse (this will not work with
  //Legions bounds checking ON)
  size_t k=0;
  for (GenericPointInRectIterator<1> pir(parent_rect); pir; pir++)
  {
   k++;
#ifndef BOUNDS_CHECKS
    double count = acc.read(DomainPoint::from_point<1>(pir.p));
    clog_assert(count==k, " ");
    std::cout<<count<<std::endl;
#endif
  }

  double * combined_data;

  LegionRuntime::Arrays::Rect<1> sr;
  LegionRuntime::Accessor::ByteOffset bo[1];
  combined_data = acc.template raw_rect_ptr<1>(parent_rect, sr, bo);

  for (size_t i=0; i<24;i++){
   clog_assert((i+1)==*(combined_data+i), " ");
  }

  return 0;
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_1,
  processor_type_t::loc, single|leaf);
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();  
 
  int num_elmts = 20;
  int num_ghost=4;

  LegionRuntime::Arrays::Rect<1> elem_rect(
      LegionRuntime::Arrays::Point<1>(0),
      LegionRuntime::Arrays::Point<1>(num_elmts+num_ghost-1));
  IndexSpace is = runtime->create_index_space(context ,
                          Domain::from_rect<1>(elem_rect));
  FieldSpace fs = runtime->create_field_space(context );
  {
    FieldAllocator allocator =
      runtime->create_field_allocator(context , fs);
    allocator.allocate_field(sizeof(double),FID_VAL);
    allocator.allocate_field(sizeof(double),FID_DERIV);
  }

  LogicalRegion stencil_lr = runtime->create_logical_region(context , is, fs);

  //Fill out LR with numbers:
  auto key_0 = __flecsi_internal_task_key(fill_task);
  Legion::TaskLauncher task_launcher(
    context_t::instance().task_id(key_0),
    Legion::TaskArgument(nullptr, 0));

  task_launcher.add_region_requirement(
      Legion::RegionRequirement(stencil_lr, READ_WRITE, EXCLUSIVE, stencil_lr)
      .add_field(FID_VAL));
  runtime->execute_task(context, task_launcher);


  LegionRuntime::Arrays::Rect<1> color_bounds(
      LegionRuntime::Arrays::Point<1>(0),LegionRuntime::Arrays::Point<1>(1-1));
  Domain color_domain = Domain::from_rect<1>(color_bounds);


  //creating exclusive, shared and ghost partitionings  
  IndexPartition ex_ip, sh_ip, gh_ip;
  {
    DomainColoring ex_coloring, sh_coloring, gh_coloring;
    int index = 0;
    // Iterate over all the colors and compute the entry
    // for both partitions for each color.
    for (int color = 0; color < 1; color++)
    {
      LegionRuntime::Arrays::Rect<1> subrect1(
        LegionRuntime::Arrays::Point<1>(0),
        LegionRuntime::Arrays::Point<1>(num_elmts-num_ghost-1));
      LegionRuntime::Arrays::Rect<1> subrect2(
        LegionRuntime::Arrays::Point<1>(num_elmts-num_ghost),
        LegionRuntime::Arrays::Point<1>(num_elmts-1));
      LegionRuntime::Arrays::Rect<1> subrect3(
        LegionRuntime::Arrays::Point<1>(num_elmts),
        LegionRuntime::Arrays::Point<1>(num_elmts+num_ghost-1));
      ex_coloring[color] = Domain::from_rect<1>(subrect1);
      sh_coloring[color] = Domain::from_rect<1>(subrect2);
      gh_coloring[color] = Domain::from_rect<1>(subrect3);
    }
    ex_ip = runtime->create_index_partition(context , is, color_domain,
                                    ex_coloring, true/*disjoint*/);
    sh_ip = runtime->create_index_partition(context , is, color_domain,
                                    sh_coloring, true/*disjoint*/);
    gh_ip = runtime->create_index_partition(context , is, color_domain,
                                    gh_coloring, false/*disjoint*/);
  }

  LogicalPartition ex_lp =
    runtime->get_logical_partition(context, stencil_lr, ex_ip);
  LogicalPartition sh_lp =
    runtime->get_logical_partition(context, stencil_lr, sh_ip);
  LogicalPartition gh_lp =
    runtime->get_logical_partition(context, stencil_lr, gh_ip);
 
  auto key_1 = __flecsi_internal_task_key(internal_task_example_1);

  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher index_launcher(
    context_t::instance().task_id(key_1),
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );
 
  index_launcher.add_region_requirement(
      RegionRequirement(ex_lp, 0/*projection ID*/,
                       READ_WRITE, EXCLUSIVE, stencil_lr));
  index_launcher.add_field(0, FID_VAL);
  index_launcher.add_region_requirement(
      RegionRequirement(sh_lp, 0/*projection ID*/,
                        READ_WRITE, EXCLUSIVE, stencil_lr));
  index_launcher.add_field(1, FID_VAL);
  index_launcher.add_region_requirement(
      RegionRequirement(gh_lp, 0/*projection ID*/,
                        READ_ONLY, EXCLUSIVE, stencil_lr));
  index_launcher.add_field(2, FID_VAL);

  index_launcher.tag=MAPPER_COMPACTED_STORAGE;
 auto fm = runtime->execute_index_space(context, index_launcher);
 fm.wait_all_results();
} // driver


} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
