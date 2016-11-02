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

//#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
//#include "flecsi/execution/execution.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/partition/init_partitions_task.h"

namespace flecsi {
namespace dmp {


parts
init_partitions(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_partitions" << std::endl; 

  using legion_domain = LegionRuntime::HighLevel::Domain;
  using legion_fid = LegionRuntime::HighLevel::FieldID;
  using legion_domain_p = LegionRuntime::HighLevel::DomainPoint;

  struct parts partitions; 
#if 1
  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];
  
#endif

#if 1
  int rank; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in init_partitions native legion task rank is " <<
       rank <<  std::endl;
    

  
  for(auto & element : ip.exclusive ) {
    std::cout << " Found exclusive elemment: " << element << 
        " on rank: " << rank << std::endl;
  }
  
  for(auto & element : ip.shared) {
    std::cout << " Found shared elemment: " << element << 
        " on rank: " << rank << std::endl;
  } 

  for(auto & element : ip.ghost) {
    std::cout << " Found ghost elemment: " << element << 
        " on rank: " << rank << std::endl;
  }

  // Now that we have the index partitioning let's push it into a logical
  // region that can be used at the top level task (once we return) to
  // allow creation of the proper index partitions 
  legion_domain dom = runtime->get_index_space_domain(ctx,
        task->regions[0].region.get_index_space());
  LegionRuntime::Arrays::Rect<2> rect = dom.get_rect<2>();
  legion_fid fid = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<
    LegionRuntime::Accessor::AccessorType::Generic, int> acc_part =
    regions[0].get_field_accessor(fid).typeify<int>();

  GenericPointInRectIterator<2> pir(rect);
  for(auto & element : ip.exclusive) {
    if(pir){
      acc_part.write(legion_domain_p::from_point<2>(pir.p), element);
      pir++;
    } else {
      abort();
    }//end if
  }//end for

  for(auto & element : ip.shared) {
    if(pir) {
       acc_part.write(legion_domain_p::from_point<2>(pir.p), element);
       pir++;
    } else {
      abort();
    }//end if
  }//end for

  for(auto & element : ip.ghost) {
    if(pir) {
      acc_part.write(legion_domain_p::from_point<2>(pir.p), element);
      pir++;
    } else {
      abort();
    }//end if
  }//end for

#endif
  partitions.exclusive = ip.exclusive.size();
  partitions.shared = ip.shared.size();
  partitions.ghost = ip.ghost.size();

  std::cout << "about to return partitions (exclusive,shared,ghost) ("
            << partitions.exclusive << "," << partitions.shared << "," 
            << partitions.ghost << ")" << std::endl;
  return partitions; 

#if 0

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_partitions" << std::endl;

//#if 1
  flecsi::execution::context_t & context_ =
             flecsi::execution::context_t::instance();
  auto array =
    context_.interop_helper_.data_storage_[0];

  using index_partition_t = index_partition__<size_t>;

  //array__<std::shared_ptr<index_partition_t>, 3> array2;
  //array2   = *array;
  index_partition_t ip = (*array)[0];
//#endif

//#if 1
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in init_partitions native legion task rank is " <<
               rank <<  std::endl;



  for(auto & element : ip.exclusive ) {
    std::cout << " Found exclusive elemment: " << element <<
               " on rank: " << rank << std::endl;
  }

  for(auto & element : ip.shared) {
    std::cout << " Found shared elemment: " << element << 
                  " on rank: " << rank << std::endl;
  }

  for(auto & element : ip.ghost) {
    std::cout << " Found ghost elemment: " << element << 
                 " on rank: " << rank << std::endl;
  }

  // Now that we have the index partitioning let's push it into a logical
  //  region that can be used at the top level task (once we return) to
  //  allow creation of the proper index partitions 
  LegionRuntime::HighLevel::Domain dom = 
               runtime->get_index_space_domain(
                ctx, task->regions[0].region.get_index_space());
  LegionRuntime::Arrays::Rect<2> rect = dom.get_rect<2>();
  LegionRuntime::HighLevel::FieldID fid =
                          *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<
    LegionRuntime::Accessor::AccessorType::Generic, int> acc_part =
            regions[0].get_field_accessor(fid).typeify<int>();
 
  GenericPointInRectIterator<2> pir(rect);
  for(auto & element : ip.exclusive) {
    if(pir)
      pir++; 
    else
      abort();
    
    acc_part.write(LegionRuntime::HighLevel::DomainPoint::from_point<2>(pir.p), element);
  }
    
#endif
}

void   
fill_cells_global_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{

  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  std::cout << "Inside of the fill_cells_global_task" << std::endl;

  using field_id = LegionRuntime::HighLevel::FieldID;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  using domain_point = LegionRuntime::HighLevel::DomainPoint;

  field_id fid_cells_glob = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, int> 
    acc_cells_glob=
    regions[0].get_field_accessor(fid_cells_glob).typeify<int>();
  legion_domain dom_cells_glob = runtime->get_index_space_domain(ctx,
      task->regions[0].region.get_index_space());
  Rect<1> rect_cells_glob = dom_cells_glob.get_rect<1>();

  field_id fid_cells_part = *(task->regions[1].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, int> 
    acc_cells_part=
    regions[1].get_field_accessor(fid_cells_part).typeify<int>();
  legion_domain dom_cells_part = runtime->get_index_space_domain(ctx,
      task->regions[1].region.get_index_space());
  Rect<2> rect_cells_part = dom_cells_part.get_rect<2>();
 
  int index =0;
  for (int i = rect_cells_glob.lo.x[0]; i<= rect_cells_glob.hi.x[0];i++){
    int proc_id= rect_cells_part.lo.x[0];
    int value = acc_cells_part.read (
        domain_point::from_point<2>(make_point(proc_id,index)));
    acc_cells_glob.write(domain_point::from_point<1>(make_point(i)), value);
    index++;
  }

}//end fill_cells_global_task


std::vector<ptr_t>
find_ghost_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  assert(regions.size() == 3);
  assert(task->regions.size() == 3);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);
  assert(task->regions[2].privilege_fields.size() == 1);
  std::cout << "Inside of the fill_ghost_id_task" << std::endl;

  std::vector<ptr_t> ghost_pointers;

  using field_id = LegionRuntime::HighLevel::FieldID;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using legion_domain = LegionRuntime::HighLevel::Domain;
  using domain_point = LegionRuntime::HighLevel::DomainPoint;

  field_id fid_ghost = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, int>
    acc_ghost=
    regions[0].get_field_accessor(fid_ghost).typeify<int>();
  legion_domain dom_ghost = runtime->get_index_space_domain(ctx,
      task->regions[0].region.get_index_space());
  Rect<1> rect_ghost = dom_ghost.get_rect<1>();

  field_id fid_cells_glob = *(task->regions[1].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, int>
    acc_cells_glob=
    regions[1].get_field_accessor(fid_cells_glob).typeify<int>();
  legion_domain dom_cells_glob = runtime->get_index_space_domain(ctx,
      task->regions[1].region.get_index_space());
  Rect<1> rect_cells_glob = dom_cells_glob.get_rect<1>();
 
  field_id fid_cells_tmp = *(task->regions[2].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<generic_type, int>
    acc_cells_tmp=
    regions[2].get_field_accessor(fid_cells_tmp).typeify<int>();
  legion_domain dom_cells_tmp = runtime->get_index_space_domain(ctx,
      task->regions[2].region.get_index_space());
  Rect<2> rect_cells_tmp = dom_cells_tmp.get_rect<2>();

  int index =0;
  assert ((rect_ghost.hi.x[0]-rect_ghost.lo.x[0])
      ==(rect_cells_tmp.hi.x[1]-rect_cells_tmp.lo.x[1]));

//use index iterator instead of GenericPointIterator
  for (int i = rect_ghost.lo.x[0]; i< rect_ghost.hi.x[0];i++){
    int j=rect_cells_tmp.lo.x[1]+index;
    int proc_id= rect_cells_tmp.lo.x[0];
    //get ghost value
    int ghost_value = acc_cells_tmp.read (
        domain_point::from_point<2>(make_point(proc_id,j)));
    //search in cells_global for this falue and store id in acc_ghost(i)
    for(GenericPointInRectIterator<1> pir(rect_cells_glob); pir; pir++){
//      ptr_t ghost_ptr = itr.next();
      int cells_value = acc_cells_glob.read (
        domain_point::from_point<1>(pir.p));
      if ( cells_value==ghost_value)
      {
        acc_ghost.write(domain_point::from_point<1>(make_point(i)), pir.p);
//        ghost_pointers.push_back(ghost_ptr);
        abort;
      }//end if
    }//end for
    index++;     
  }//end for
 
 return ghost_pointers;
}


} // namespace dmp
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

