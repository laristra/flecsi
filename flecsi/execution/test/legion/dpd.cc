#include <cinchtest.h>
#include <iostream>
#include <sstream>

#include "flecsi/execution/legion/dpd.h"

#include "legion.h"
#include "legion_config.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

using namespace std;

using namespace Legion;
using namespace LegionRuntime::Accessor;

using namespace flecsi;
using namespace execution;

enum TaskIDs{
  TOP_LEVEL_TID
};

using entity_id = size_t;
using partition_id = size_t;

using partition_vec = vector<entity_id>;
using connectivity_vec = legion_dpd::connectivity_vec;

const size_t WIDTH = 4;
const size_t HEIGHT = 4;
const size_t NUM_PARTITIONS = 4;

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){

  legion_helper h(runtime, ctx);

  size_t num_cells = WIDTH * HEIGHT;

  size_t from_dim = 2;
  size_t to_dim = 0;

  set<entity_id> vertex_set;

  size_t total_conns = 0;

  partition_vec cells(num_cells);
  partition_vec vertices;

  size_t partition_size = num_cells / NUM_PARTITIONS;

  connectivity_vec connectivity(num_cells);

  for(size_t j = 0; j < HEIGHT; ++j){
    for(size_t i = 0; i < WIDTH; ++i){
      size_t c = j * WIDTH + i;

      size_t p = c / partition_size;

      cells[c] = p;
      
      auto& cvc = connectivity[c];

      entity_id v1 = j * (WIDTH + 1) + i;
      entity_id v2 = j * (WIDTH + 1) + i + 1;
      entity_id v3 = (j + 1) * (WIDTH + 1) + i;
      entity_id v4 = (j + 1) * (WIDTH + 1) + i + 1;

      if(vertex_set.insert(v1).second){
        if(v1 >= vertices.size()){
          vertices.resize(v1 + 1);
        }

        vertices[v1] = p;
      }

      if(vertex_set.insert(v2).second){
        if(v2 >= vertices.size()){
          vertices.resize(v2 + 1);
        }

        vertices[v2] = p;
      }

      if(vertex_set.insert(v3).second){
        if(v3 >= vertices.size()){
          vertices.resize(v3 + 1);
        }

        vertices[v3] = p;
      }

      if(vertex_set.insert(v4).second){
        if(v4 >= vertices.size()){
          vertices.resize(v4 + 1);
        }

        vertices[v4] = p;
      }

      cvc.push_back(v1); 
      cvc.push_back(v2); 
      cvc.push_back(v3); 
      cvc.push_back(v4);

      total_conns += cvc.size(); 
    }
  }

  size_t num_vertices = vertices.size();

  legion_dpd::partitioned_unstructured cells_part;

  {
    cells_part.size = num_cells;

    IndexSpace is = h.create_index_space(cells_part.size);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t c = 0; c < cells.size(); ++c){
      size_t p = cells[c]; 

      ptr_t ptr = ia.alloc(1);
      coloring[p].points.insert(ptr);
      ++cells_part.count_map[p];
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), legion_dpd::ENTITY_FID);
    
    fa.allocate_field(sizeof(legion_dpd::ptr_count),
                      legion_dpd::connectivity_field_id(from_dim, to_dim));

    cells_part.lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cells_part.lr, WRITE_DISCARD, 
      EXCLUSIVE, cells_part.lr);
    
    rr.add_field(legion_dpd::ENTITY_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = 
      pr.get_field_accessor(legion_dpd::ENTITY_FID).typeify<entity_id>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t c = 0; c < cells.size(); ++c){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      ac.write(ptr, c);
    }

    cells_part.ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  legion_dpd::partitioned_unstructured vertices_part;

  {
    IndexSpace is = h.create_index_space(num_vertices);
    vertices_part.size = num_vertices;

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t v = 0; v < vertices.size(); ++v){
      size_t p = vertices[v];
      ptr_t ptr = ia.alloc(1);
      coloring[p].points.insert(ptr);
      ++vertices_part.count_map[p];
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), legion_dpd::ENTITY_FID);

    vertices_part.lr = h.create_logical_region(is, fs);

    RegionRequirement rr(vertices_part.lr, WRITE_DISCARD, 
      EXCLUSIVE, vertices_part.lr);
    rr.add_field(legion_dpd::ENTITY_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = 
      pr.get_field_accessor(legion_dpd::ENTITY_FID).typeify<entity_id>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t v = 0; v < vertices.size(); ++v){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      ac.write(ptr, v);
    }
    
    vertices_part.ip = 
      runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  legion_dpd::partitioned_unstructured raw_connectivity_part;

  {

    IndexSpace is = h.create_index_space(total_conns);
    raw_connectivity_part.size = total_conns;

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(legion_dpd::entity_pair),
                      legion_dpd::ENTITY_PAIR_FID);

    Coloring coloring;

    for(entity_id c = 0; c < num_cells; ++c){
      size_t p = cells[c];
      auto& cvc = connectivity[c];
      size_t n = cvc.size();
      raw_connectivity_part.count_map[p] += n;
      
      for(size_t j = 0; j < n; ++j){
        ptr_t ptr = ia.alloc(1);
        coloring[p].points.insert(ptr);    
      }
    }

    raw_connectivity_part.lr = h.create_logical_region(is, fs);

    RegionRequirement rr(raw_connectivity_part.lr, 
      WRITE_DISCARD, EXCLUSIVE, raw_connectivity_part.lr);
    rr.add_field(legion_dpd::ENTITY_PAIR_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = pr.get_field_accessor(legion_dpd::ENTITY_PAIR_FID).typeify<
      legion_dpd::entity_pair>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t c = 0; c < cells.size(); ++c){
      auto& cvc = connectivity[c];
      size_t n = cvc.size();

      for(size_t j = 0; j < n; ++j){
        assert(itr.has_next());
        ptr_t ptr = itr.next();
        legion_dpd::entity_pair p;
        p.e1 = c;
        p.e2 = cvc[j];
        ac.write(ptr, p);        
      }
    }

    raw_connectivity_part.ip = 
      runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  legion_dpd dpd(ctx, runtime);
  dpd.create_connectivity(from_dim, cells_part, to_dim, vertices_part, 
    raw_connectivity_part);
  dpd.dump(from_dim, to_dim);
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TID,
    Processor::LOC_PROC, true, false);

  Runtime::register_legion_task<legion_dpd::init_connectivity_task>(legion_dpd::INIT_CONNECTIVITY_TID,
    Processor::LOC_PROC, false, true);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
