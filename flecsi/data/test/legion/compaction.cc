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

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
  INDEX_TASK_ID
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

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context context, Runtime* runtime){

  legion_helper h(runtime, context);

  FieldSpace fs = h.create_field_space();

  FieldAllocator a = h.create_field_allocator(fs);
  a.allocate_field(sizeof(double), 0);

  IndexSpace is = h.create_index_space(0, 10);

  LogicalRegion lr = h.create_logical_region(is, fs);

  DomainColoring dc;

  Domain d1 = h.domain_from_rect(0, 4);
  Domain d2 = h.domain_from_rect(5, 9);
  
  dc[0] = d1;
  dc[1] = d2;

  Domain cd = h.domain_from_rect(0, 1);

  IndexPartition ip = 
    runtime->create_index_partition(context, is, cd, dc, true);

  RegionRequirement rr(lr, WRITE_DISCARD, EXCLUSIVE, lr);
  
  rr.add_field(0);

  InlineLauncher il(rr);

  PhysicalRegion pr = runtime->map_region(context, il);

  double* buf;
  h.get_buffer(pr, buf, 0);

  for(size_t i = 0; i < 10; ++i){
    buf[i] = i * 10;
  }

  runtime->unmap_region(context, pr);

  LogicalPartition lp =
    runtime->get_logical_partition(context, lr, ip);

  ArgumentMap arg_map;

  IndexLauncher il2(INDEX_TASK_ID, cd, TaskArgument(nullptr, 0), arg_map);

  il2.add_region_requirement(
        RegionRequirement(lp, 0, READ_ONLY, EXCLUSIVE, lr));

  il2.region_requirements[0].add_field(0);

  FutureMap fm = h.execute_index_space(il2);
  fm.wait_all_results();

}

void index_task(const Task* task,
                const std::vector<PhysicalRegion>& regions,
                Context context, Runtime* runtime){
  auto ac = 
    regions[0].get_field_accessor(0).typeify<double>();
  LogicalRegion lr = regions[0].get_logical_region();
  IndexSpace is = lr.get_index_space();
  Domain domain = runtime->get_index_space_domain(context, is);
  LegionRuntime::Arrays::Rect<1> r = domain.get_rect<1>();
  LegionRuntime::Arrays::Rect<1> sr;
  LegionRuntime::Accessor::ByteOffset bo[1];
  double* data = ac.template raw_rect_ptr<1>(r, sr, bo);
  size_t size = r.hi;

  for(size_t i = 0; i < size; ++i){
    np(i);
    np(data[i]);
  }  
}

TEST(legion, compaction) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
    Processor::LOC_PROC, true/*single*/, false/*index*/);

  Runtime::register_legion_task<index_task>(INDEX_TASK_ID,
    Processor::LOC_PROC, false/*single*/, true/*index*/);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
