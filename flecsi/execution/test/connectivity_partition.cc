#include <cinchtest.h>
#include <iostream>
#include <sstream>
#include <cereal/archives/binary.hpp>

#include "flecsi/topology/mesh_topology.h"

#include "legion.h"
#include "legion_config.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

using namespace std;

using namespace Legion;
using namespace LegionRuntime::Accessor;

using namespace flecsi;
using namespace topology;

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

size_t equilikely(size_t a, size_t b){
  return uniform(a, b + 1.0);
}

enum TaskIDs{
  TOP_LEVEL_TID
};

enum FieldIDs{
  ENTITY_FID,
  PTR_FID
};

class legion_helper{
public:
  legion_helper(Runtime* runtime, Context context)
  : runtime_(runtime),
  context_(context){}

  // structured
  IndexSpace create_index_space(unsigned start, unsigned end){
    assert(end >= start);
    Rect<1> rect(Point<1>(start), Point<1>(end - 0));
    return runtime_->create_index_space(context_, Domain::from_rect<1>(rect));  
  }

  DomainPoint domain_point(size_t p){
    return DomainPoint::from_point<1>(make_point(p));
  }

  Domain domain_from_point(size_t p){
    Rect<1> rect(Point<1>(p), Point<1>(p - 0));
    return Domain::from_rect<1>(rect);
  }

  Domain domain_from_rect(size_t start, size_t end){
    Rect<1> rect(Point<1>(start), Point<1>(end - 0));
    return Domain::from_rect<1>(rect);
  }

  // unstructured
  IndexSpace create_index_space(size_t n) const{
    assert(n > 0);
    return runtime_->create_index_space(context_, n);  
  }

  FieldSpace create_field_space() const{
    return runtime_->create_field_space(context_);
  }

  FieldAllocator create_field_allocator(FieldSpace fs) const{
    return runtime_->create_field_allocator(context_, fs);
  }

  LogicalRegion create_logical_region(IndexSpace is, FieldSpace fs) const{
    return runtime_->create_logical_region(context_, is, fs);
  }

  Future execute_task(TaskLauncher l) const{
    return runtime_->execute_task(context_, l);
  }

  Domain get_index_space_domain(IndexSpace is) const{
    return runtime_->get_index_space_domain(context_, is);
  }

  DomainPoint domain_point(size_t i) const{
    return DomainPoint::from_point<1>(Point<1>(i)); 
  }

  FutureMap execute_index_space(IndexLauncher l) const{
    return runtime_->execute_index_space(context_, l);
  }

  IndexAllocator create_index_allocator(IndexSpace is) const{
    return runtime_->create_index_allocator(context_, is);
  }

  Domain get_domain(PhysicalRegion pr) const{
    LogicalRegion lr = pr.get_logical_region();
    IndexSpace is = lr.get_index_space();
    return runtime_->get_index_space_domain(context_, is);     
  }

  template<class T>
  void get_buffer(PhysicalRegion pr, T*& buf, size_t field = 0) const{
    auto ac = pr.get_field_accessor(field).typeify<T>();
    Domain domain = get_domain(pr); 
    Rect<1> r = domain.get_rect<1>();
    Rect<1> sr;
    ByteOffset bo[1];
    buf = ac.template raw_rect_ptr<1>(r, sr, bo);
  }

  char* get_raw_buffer(PhysicalRegion pr, size_t field = 0) const{
    auto ac = pr.get_field_accessor(field).typeify<char>();
    Domain domain = get_domain(pr); 
    Rect<1> r = domain.get_rect<1>();
    Rect<1> sr;
    ByteOffset bo[1];
    return ac.template raw_rect_ptr<1>(r, sr, bo);
  }

  void unmap_region(PhysicalRegion pr) const{
    runtime_->unmap_region(context_, pr);
  }

private:
  mutable Runtime* runtime_;
  mutable Context context_;
};

using entity_id = size_t;
using partition_id = size_t;

using connectivity_vec = vector<vector<entity_id>>;
using partition_vec = vector<entity_id>;

const size_t WIDTH = 4;
const size_t HEIGHT = 4;
const size_t NUM_PARTITIONS = 4;

// develop how mesh connectivity can be represented/traversed 
// using a test connectivity of cells -> vertices using a 
// uniform 2d quad mesh and unstructured Logical Regions using 
// a CRS-like (compressed row storage) scheme 

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){

  legion_helper h(runtime, ctx);

  // create sample connectivity raw data in terms of (simulated)
  // FleCSI entity id

  size_t num_cells = WIDTH * HEIGHT;

  connectivity_vec connectivity(num_cells);

  set<entity_id> vertex_set;

  size_t total_conns = 0;

  partition_vec cells(num_cells);
  partition_vec vertices;

  size_t partition_size = num_cells / NUM_PARTITIONS;

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

  // create cells LR - this is an unstructured index space that maps local
  // unstructured id to (simulated) FleCSI entity id

  LogicalRegion cells_lr;
  IndexPartition cells_ip;

  vector<ptr_t> cell_ptrs(num_cells);

  {

    IndexSpace is = h.create_index_space(num_cells);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t c = 0; c < cells.size(); ++c){
      size_t p = cells[c]; 
      
      ptr_t ptr = ia.alloc(1);
      cell_ptrs[c] = ptr;
      coloring[p].points.insert(ptr); 
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), ENTITY_FID);

    cells_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cells_lr, WRITE_DISCARD, EXCLUSIVE, cells_lr);
    rr.add_field(ENTITY_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = pr.get_field_accessor(ENTITY_FID).typeify<entity_id>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t c = 0; c < cells.size(); ++c){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      ac.write(ptr, c);
    }

    cells_ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  // create vertices LR - this is an unstructured index space that maps local
  // unstructured id to (simulated) FleCSI entity id

  LogicalRegion vertices_lr;
  IndexPartition vertices_ip;

  vector<ptr_t> vertex_ptrs(num_vertices);

  {

    IndexSpace is = h.create_index_space(num_vertices);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t v = 0; v < vertices.size(); ++v){
      size_t p = vertices[v];
      ptr_t ptr = ia.alloc(1);
      vertex_ptrs[v] = ptr;
      coloring[p].points.insert(ptr); 
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), ENTITY_FID);

    vertices_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(vertices_lr, WRITE_DISCARD, EXCLUSIVE, vertices_lr);
    rr.add_field(ENTITY_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = pr.get_field_accessor(ENTITY_FID).typeify<entity_id>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t v = 0; v < vertices.size(); ++v){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      ac.write(ptr, v);
    }
    
    vertices_ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  // create the 'to' side of the connectivity - using a CRS-like scheme - 
  // this is an unstructured index space that stores ptr_t entries 
  // that points to entries in the vertices unstructured LR

  LogicalRegion cv_to_lr;
  IndexPartition cv_to_ip;

  vector<ptr_t> start_ptrs(num_cells);

  {

    IndexSpace is = h.create_index_space(total_conns);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t c = 0; c < cells.size(); ++c){
      size_t p = cells[c]; 
      
      const auto& cvc = connectivity[c];

      ptr_t ptr = ia.alloc(cvc.size());
      start_ptrs[c] = ptr;
      coloring[p].points.insert(ptr); 
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(ptr_t), PTR_FID);

    cv_to_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cv_to_lr, WRITE_DISCARD, EXCLUSIVE, cv_to_lr);
    rr.add_field(PTR_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = pr.get_field_accessor(PTR_FID).typeify<ptr_t>();

    IndexIterator itr(runtime, ctx, is);
    
    for(size_t c = 0; c < cells.size(); ++c){
      const auto& cvc = connectivity[c]; 

      size_t n = cvc.size();

      for(size_t j = 0; j < n; ++j){
        assert(itr.has_next());
        ptr_t ptr = itr.next();
        size_t v = cvc[j];
        ac.write(ptr, vertex_ptrs[v]);  
      }
    }

    cv_to_ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  // create the 'from' side of the connectivity - using a CRS-like scheme - 
  // this is an structured index space that has entries 
  // e[i] and e[i + 1] as ptr_t that point to the 'to' LR that 
  // correspond to the start of the 'to' side (e[i]) and the end of 
  // this range (e[i + 1])

  LogicalRegion cv_from_lr;

  {
    IndexSpace is = h.create_index_space(0, num_cells);

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(ptr_t), PTR_FID);

    cv_from_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cv_from_lr, WRITE_DISCARD, EXCLUSIVE, cv_from_lr);
    rr.add_field(PTR_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    ptr_t* buf;
    h.get_buffer(pr, buf, PTR_FID);
    for(size_t c = 0; c < cells.size(); ++c){
      buf[c] = start_ptrs[c];
    }

    runtime->unmap_region(ctx, pr);
  }

  // test code to simulate traversals:
  //
  // foreach(cell c){
  //   print c.id
  //   foreach(vertex v in c){
  //     print v.id
  //   }
  // }

  {
    RegionRequirement rr1(cv_from_lr, READ_ONLY, EXCLUSIVE, cv_from_lr);
    rr1.add_field(PTR_FID);
    InlineLauncher il1(rr1);

    PhysicalRegion pr1 = runtime->map_region(ctx, il1);

    pr1.wait_until_valid();

    RegionRequirement rr2(cv_to_lr, READ_ONLY, EXCLUSIVE, cv_to_lr);
    rr2.add_field(PTR_FID);
    InlineLauncher il2(rr2);

    PhysicalRegion pr2 = runtime->map_region(ctx, il2);

    pr2.wait_until_valid();

    RegionRequirement rr3(vertices_lr, READ_ONLY, EXCLUSIVE, vertices_lr);
    rr3.add_field(ENTITY_FID);
    InlineLauncher il3(rr3);

    PhysicalRegion pr3 = runtime->map_region(ctx, il3);

    pr3.wait_until_valid();

    for(entity_id c = 0; c < num_cells; ++c){
      cout << "-------- cell: " << c << endl; 

      ptr_t* buf;
      h.get_buffer(pr1, buf, PTR_FID);
      ptr_t to_start = buf[c];
      ptr_t to_end = buf[c + 1];

      IndexIterator itr(runtime, ctx, cv_to_lr.get_index_space(), to_start);

      auto ac = pr2.get_field_accessor(PTR_FID).typeify<ptr_t>();
      auto ac2 = pr3.get_field_accessor(ENTITY_FID).typeify<entity_id>();

      ptr_t ptr = itr.next();

      while(ptr != to_end){
        ptr_t vertex_ptr = ac.read(ptr);
        entity_id vertex_id = ac2.read(vertex_ptr);

        cout << "--- vertex: " << vertex_id << endl;
        
        if(!itr.has_next()){
          break;
        }

        ptr = itr.next();
      }
    }
  }
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TID,
    Processor::LOC_PROC, true, false);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
