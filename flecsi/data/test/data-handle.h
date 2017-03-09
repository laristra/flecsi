/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_test_h
#define flecsi_data_handle_test_h

#define DH1 1
#undef flecsi_execution_legion_task_wrapper_h
#include "flecsi/execution/legion/task_wrapper.h"

#include <iostream>
#include <vector>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/legion/data_policy.h"
#include "flecsi/execution/legion/helper.h"
#include "flecsi/execution/task_ids.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

///
// \file data-handle.h
// \authors nickm
// \date Initial file creation: Jan 25, 2017
///

using namespace std;
using namespace flecsi;
using namespace execution;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

template<typename T>
using accessor_t = flecsi::data::legion::dense_accessor_t<T, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;

namespace flecsi {
namespace execution {
  
void task1(accessor_t<double> x) {
  np(x[0]);
  np(x[1]);
} // task1

flecsi_register_task(task1, loc, single);

const size_t NUM_CELLS = 16;
const size_t NUM_PARTITIONS = 4;

class data_client : public data::data_client_t{
public:
  size_t indices(size_t index_space) const override{
    return NUM_CELLS;
  }
};

using entity_id = size_t;
using partition_id = size_t;

using partition_vec = vector<entity_id>;

void
specialization_driver(
  int argc, 
  char ** argv
)
{
  std::cout << "driver start" << std::endl;

  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);

  field_ids_t & fid_t = field_ids_t::instance();

  legion_helper h(runtime, context);

  partition_vec cells(NUM_CELLS);
  partition_vec cells_shared(NUM_CELLS, 999);
  partition_vec cells_ghost(NUM_CELLS, 999);

  size_t partition_size = NUM_CELLS / NUM_PARTITIONS;

  context_t::partitioned_index_space cells_part;

  for(size_t c = 0; c < NUM_CELLS; ++c){
    size_t p = c / partition_size;

    cells[c] = p;

    if(c % partition_size == partition_size - 1 && c < NUM_CELLS - 1){
      cells_shared[c] = p;
      ++cells_part.shared_count_map[p];

      cells_ghost[c + 1] = p + 1;
      ++cells_part.shared_count_map[p + 1];
    }
  }

  {
    cells_part.size = NUM_CELLS;

    IndexSpace is = h.create_index_space(cells_part.size);

    IndexAllocator ia = runtime->create_index_allocator(context, is);
    
    Coloring coloring;
    Coloring shared_coloring;
    Coloring ghost_coloring;

    for(size_t c = 0; c < cells.size(); ++c){
      size_t p = cells[c]; 
      ++cells_part.exclusive_count_map[p];
      
      if(cells_shared[c] < NUM_PARTITIONS){
        ++cells_part.shared_count_map[p];
      }

      if(cells_ghost[c] < NUM_PARTITIONS){
        ++cells_part.ghost_count_map[p];
      }
    }

    for(auto& itr : cells_part.exclusive_count_map){
      size_t p = itr.first;
      int count = itr.second;
      ptr_t start = ia.alloc(count);

      for(int i = 0; i < count; ++i){
        coloring[p].points.insert(start + i);
      }
    }

    for(auto& itr : cells_part.shared_count_map){
      size_t p = itr.first;
      int count = itr.second;
      ptr_t start = ia.alloc(count);

      for(int i = 0; i < count; ++i){
        shared_coloring[p].points.insert(start + i);
      }
    }

    for(auto& itr : cells_part.ghost_count_map){
      size_t p = itr.first;
      int count = itr.second;
      ptr_t start = ia.alloc(count);

      for(int i = 0; i < count; ++i){
        ghost_coloring[p].points.insert(start + i);
      }
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), fid_t.fid_entity);
    
    cells_part.entities_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cells_part.entities_lr, WRITE_DISCARD, 
      EXCLUSIVE, cells_part.entities_lr);
    
    rr.add_field(fid_t.fid_entity);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(context, il);

    pr.wait_until_valid();

    auto ac = 
      pr.get_field_accessor(fid_t.fid_entity).typeify<entity_id>();

    IndexIterator itr(runtime, context, is);
    ptr_t ptr = itr.next();

    for(size_t c = 0; c < cells.size(); ++c){
      ac.write(ptr, c);
      ++ptr;
    }

    cells_part.exclusive_ip = 
      runtime->create_index_partition(context, is, coloring, true);

    cells_part.shared_ip = 
      runtime->create_index_partition(context, is, shared_coloring, true);

    cells_part.ghost_ip = 
      runtime->create_index_partition(context, is, ghost_coloring, false);

    runtime->unmap_region(context, pr);
  }

  data_client dc;

  dc.put_index_space(0, cells_part);

  context_.add_index_space("partition1","cells", cells_part);

  execution::mpilegion_context_policy_t::partitioned_index_space& is3=
      context_.get_index_space("partition1","cells");

  
  // data client
  // "hydro" namespace
  // "pressure" name
  // type double
  // dense storage type
  // versions
  // index space

  flecsi_register_data(dc, hydro, pressure, double, dense, 1, 0);

  {
    auto ac = 
      flecsi_get_accessor(dc, hydro, pressure, double, dense, /* version */ 0);

    ac[0] = 100.0;
    ac[1] = 200.0;
  }

  auto h1 = 
    flecsi_get_handle(dc, hydro, pressure, double, dense, 0, rw, rw, ro);

  flecsi_execute_task(task1, loc, single, h1);

} // driver

} // namespace execution
} // namespace flecsi

//#undef DH1

#endif // flecsi_data_handle_test_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
