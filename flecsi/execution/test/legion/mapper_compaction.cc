/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/execution/legion/internal_task.h"
#include "cinchtest.h"

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

// Define a Legion task to register.
int internal_task_example_1(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context context,
  Legion::Runtime * runtime) 
{
  std::cout <<"inside of the task1" <<std::endl;
} // internal_task_example

// Register the task. The task id is automatically generated.
__flecsi_internal_register_legion_task(internal_task_example_1,
  processor_type_t::loc, single);

void driver(int argc, char ** argv) {

#if defined(ENABLE_LEGION_TLS)
  auto runtime = Legion::Runtime::get_runtime();
  auto context = Legion::Runtime::get_context();  
#else
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);
#endif
 
  int num_elements = 1024;
  int num_subregions = 4;

  Rect<1> elem_rect(Point<1>(0),Point<1>(num_elements-1));
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
  Rect<1> color_bounds(Point<1>(0),Point<1>(num_subregions-1));
  Domain color_domain = Domain::from_rect<1>(color_bounds);

  IndexPartition ex_ip, sh_ip, gh_ip;
  {
    const int lower_bound = num_elements/num_subregions;
    const int upper_bound = lower_bound+1;
    const int number_small = num_subregions - (num_elements % num_subregions);
    DomainColoring ex_coloring, sh_coloring, gh_coloring;
    int index = 0;
    // Iterate over all the colors and compute the entry
    // for both partitions for each color.
    for (int color = 0; color < num_subregions; color++)
    {
      int num_elmts = color < number_small ? lower_bound : upper_bound;
      assert((index+num_elmts) <= num_elements);
      Rect<1> subrect1(Point<1>(index),Point<1>(index+num_elmts-4));
      Rect<1> subrect2(Point<1>(index+num_elmts-4),Point<1>(index+num_elmts-1));
      ex_coloring[color] = Domain::from_rect<1>(subrect1);
      sh_coloring[color] = Domain::from_rect<1>(subrect2);
      // Now compute the points assigned to this color for
      // the second partition.  Here we need a superset of the
      // points that we just computed including the two additional
      // points on each side.  We handle the edge cases by clamping
      // values to their minimum and maximum values.  This creates
      // four cases of clamping both above and below, clamping below,
      // clamping above, and no clamping.
      if (index < 2)
      {
        if ((index+num_elmts+2) > num_elements)
        {
          // Clamp both
          Rect<1> ghost_rect(Point<1>(0),Point<1>(num_elements-1));
          gh_coloring[color] = Domain::from_rect<1>(ghost_rect);
        }
        else
        {
          // Clamp below
          Rect<1> ghost_rect(Point<1>(0),Point<1>(index+num_elmts+1));
          gh_coloring[color] = Domain::from_rect<1>(ghost_rect);
        }
      }
      else
      {
        if ((index+num_elmts+2) > num_elements)
        {
          // Clamp above
          Rect<1> ghost_rect(Point<1>(index-2),Point<1>(num_elements-1));
          gh_coloring[color] = Domain::from_rect<1>(ghost_rect);
        }
        else
        {
          // Normal case
          Rect<1> ghost_rect(Point<1>(index-2),Point<1>(index+num_elmts+1));
          gh_coloring[color] = Domain::from_rect<1>(ghost_rect);
        }
      }
      index += num_elmts;
    }
    // Once we've computed both of our colorings then we can
    // create our partitions.  Note that we tell the runtime
    // that one is disjoint will the second one is not.
    ex_ip = runtime->create_index_partition(context , is, color_domain,
                                    ex_coloring, true/*disjoint*/);
    sh_ip = runtime->create_index_partition(context , is, color_domain,
                                    sh_coloring, true/*disjoint*/);
    gh_ip = runtime->create_index_partition(context , is, color_domain,
                                    gh_coloring, false/*disjoint*/);
  }
 
  auto key_1 = __flecsi_internal_task_key(internal_task_example_1);


  Legion::ArgumentMap arg_map;
  Legion::IndexLauncher index_launcher(
    context_t::instance().task_id(key_1),
    Legion::Domain::from_rect<1>(context_t::instance().all_processes()),
    Legion::TaskArgument(0, 0),
    arg_map
  );
   
  //index_launcher.tag=MAPPER_FORCE_RANK_MATCH;
 auto fm = runtime->execute_index_space(context, index_launcher);
 fm.wait_all_results();
} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
