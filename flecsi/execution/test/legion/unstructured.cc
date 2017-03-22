#include <cinchtest.h>
#include <iostream>
#include <sstream>

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

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
  INIT_TASK_ID,
  READ_TASK_ID
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

struct metadata{
  ptr_t ptr;
  size_t size;
};

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){

  legion_helper h(runtime, ctx);

  IndexSpace is = h.create_index_space(2048);

  IndexAllocator ia = runtime->create_index_allocator(ctx, is);

  std::vector<metadata> mds;

  for(size_t i = 0; i < 10; ++i){
    metadata md;
    md.size = 50 + i * 10;
    md.ptr = ia.alloc(md.size);
    mds.push_back(md);
  }

  FieldSpace fs = h.create_field_space();

  FieldAllocator a = h.create_field_allocator(fs);
  a.allocate_field(sizeof(int), 0);

  LogicalRegion lr = h.create_logical_region(is, fs);

  Coloring coloring;

  IndexIterator itr(runtime, ctx, is);

  ArgumentMap arg_map;

  for(size_t i = 0; i < mds.size(); ++i){
    const metadata& md = mds[i];

    for(size_t j = 0; j < md.size; ++j){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      coloring[i].points.insert(ptr);
    }

    arg_map.set_point(h.domain_point(i),
      TaskArgument(&md, sizeof(md)));
  }

  IndexPartition ip = 
    runtime->create_index_partition(ctx, is, coloring, true);

  LogicalPartition lp =
    runtime->get_logical_partition(ctx, lr, ip);

  Domain d = h.domain_from_rect(0, 9);

  IndexLauncher il(INIT_TASK_ID, d,
    TaskArgument(nullptr, 0), arg_map);

  il.add_region_requirement(
        RegionRequirement(lp, 0, 
                          WRITE_DISCARD, EXCLUSIVE, lr));
  
  il.region_requirements[0].add_field(0);

  FutureMap fm = h.execute_index_space(il);
  fm.wait_all_results();

  IndexLauncher il2(READ_TASK_ID, d,
    TaskArgument(nullptr, 0), arg_map);

  il2.add_region_requirement(
        RegionRequirement(lp, 0, 
                          READ_ONLY, EXCLUSIVE, lr));
  
  il2.region_requirements[0].add_field(0);

  FutureMap fm2 = h.execute_index_space(il2);
  fm2.wait_all_results();  
}

void init_task(const Task* task,
               const std::vector<PhysicalRegion>& regions,
               Context ctx, Runtime* runtime){

  metadata* md = (metadata*)task->local_args;

  LogicalRegion lr = regions[0].get_logical_region();
  IndexSpace is = lr.get_index_space();

  IndexIterator itr(runtime, ctx, is);

  auto ac = regions[0].get_field_accessor(0).typeify<int>();

  for(size_t i = 0; i < md->size; ++i){
    assert(itr.has_next());
    
    ptr_t ptr = itr.next();
    ac.write(ptr, i);
  }
}

void read_task(const Task* task,
               const std::vector<PhysicalRegion>& regions,
               Context ctx, Runtime* runtime){
  
  metadata* md = (metadata*)task->local_args;

  LogicalRegion lr = regions[0].get_logical_region();
  IndexSpace is = lr.get_index_space();

  IndexIterator itr(runtime, ctx, is);

  auto ac = regions[0].get_field_accessor(0).typeify<int>();

  for(size_t i = 0; i < md->size; ++i){
    assert(itr.has_next());
    
    ptr_t ptr = itr.next();
    int x = ac.read(ptr);
    np(x);
  }
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
    Processor::LOC_PROC, true, false);

  Runtime::register_legion_task<init_task>(INIT_TASK_ID,
    Processor::LOC_PROC, false, true);

  Runtime::register_legion_task<read_task>(READ_TASK_ID,
    Processor::LOC_PROC, false, true);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
