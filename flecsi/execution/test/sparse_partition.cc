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
  EV_FID,
  PTR_COUNT_FID
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

const size_t NUM_CELLS = 16;
const size_t NUM_PARTITIONS = 4;

using entity_id = size_t;
using partition_id = size_t;

using partition_vec = vector<entity_id>;

template<class T>
struct entry_value{
  size_t entry;
  T value;
};

struct ptr_count{
  ptr_t ptr;
  size_t count;
};

using value_t = double;

template<class T>
using ev_partion_vec = vector<vector<entry_value<T>>>;

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){

  legion_helper h(runtime, ctx);

  size_t partition_size = NUM_CELLS / NUM_PARTITIONS;
  size_t total_values = 0;

  partition_vec cells(NUM_CELLS);
  ev_partion_vec<value_t> entry_values(NUM_CELLS);

  for(entity_id c = 0; c < NUM_CELLS; ++c){
    partition_id p = c / partition_size;
    cells[c] = p;
    size_t n = equilikely(0, 5);

    auto& evc = entry_values[c];
    evc.resize(n);

    size_t e = 0;

    for(size_t j = 0; j < n; ++j){
      evc[j].value = uniform(0, 1);
      e = equilikely(e, e + 5);
      evc[j].entry = e;
    }

    total_values += n;
  }

  LogicalRegion cells_lr;
  IndexPartition cells_ip;

  vector<ptr_t> cell_ptrs(NUM_CELLS);

  {

    IndexSpace is = h.create_index_space(NUM_CELLS);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(entity_id c = 0; c < cells.size(); ++c){
      partition_id p = cells[c]; 
      
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
    
    for(entity_id c = 0; c < cells.size(); ++c){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      ac.write(ptr, c);
    }

    cells_ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }


  LogicalRegion ev_lr;
  IndexPartition ev_ip;

  vector<ptr_count> start_ptr_counts(NUM_CELLS);

  {

    IndexSpace is = h.create_index_space(total_values);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(entity_id c = 0; c < cells.size(); ++c){
      partition_id p = cells[c]; 
      
      const auto& evc = entry_values[c];

      size_t n = evc.size();

      ptr_t ptr = ia.alloc(n);
      start_ptr_counts[c] = ptr_count{ptr, n};
      coloring[p].points.insert(ptr); 
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entry_value<value_t>), EV_FID);

    ev_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(ev_lr, WRITE_DISCARD, EXCLUSIVE, ev_lr);
    rr.add_field(EV_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = pr.get_field_accessor(EV_FID).typeify<entry_value<value_t>>();

    IndexIterator itr(runtime, ctx, is);
    
    for(entity_id c = 0; c < cells.size(); ++c){
      const auto& evc = entry_values[c]; 

      size_t n = evc.size();

      for(size_t j = 0; j < n; ++j){
        assert(itr.has_next());
        ptr_t ptr = itr.next();
        ac.write(ptr, evc[j]);  
      }
    }

    ev_ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  LogicalRegion indices_lr;

  {
    IndexSpace is = h.create_index_space(0, NUM_CELLS);

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(ptr_count), PTR_COUNT_FID);

    indices_lr = h.create_logical_region(is, fs);

    RegionRequirement rr(indices_lr, WRITE_DISCARD, EXCLUSIVE, indices_lr);
    rr.add_field(PTR_COUNT_FID);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    ptr_count* buf;
    h.get_buffer(pr, buf, PTR_COUNT_FID);
    for(entity_id c = 0; c < cells.size(); ++c){
      buf[c] = start_ptr_counts[c];
    }

    runtime->unmap_region(ctx, pr);
  }

  {
    RegionRequirement rr1(indices_lr, READ_ONLY, EXCLUSIVE, indices_lr);
    rr1.add_field(PTR_COUNT_FID);
    InlineLauncher il1(rr1);

    PhysicalRegion pr1 = runtime->map_region(ctx, il1);

    pr1.wait_until_valid();

    RegionRequirement rr2(ev_lr, READ_ONLY, EXCLUSIVE, ev_lr);
    rr2.add_field(EV_FID);
    InlineLauncher il2(rr2);

    PhysicalRegion pr2 = runtime->map_region(ctx, il2);

    pr2.wait_until_valid();

    for(entity_id c = 0; c < cells.size(); ++c){
      cout << "-------- cell: " << c << endl; 

      ptr_count* buf;
      h.get_buffer(pr1, buf, PTR_COUNT_FID);
      const ptr_count& pc = buf[c];

      IndexIterator itr(runtime, ctx, ev_lr.get_index_space(), pc.ptr);

      auto ac = 
        pr2.get_field_accessor(EV_FID).typeify<entry_value<value_t>>();

      size_t i = 0;
      size_t n = pc.count;

      while(i < n){
        assert(itr.has_next());
        ptr_t ptr = itr.next();

        const entry_value<value_t>& ev = ac.read(ptr);

        cout << "--- entry: " << ev.entry << endl;
        cout << "--- value: " << ev.value << endl;
        
        ++i;
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
