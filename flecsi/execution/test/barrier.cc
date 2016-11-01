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
  TEST1_TASK_ID,
  TEST2_TASK_ID
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

struct test_args{
  int ts;
  PhaseBarrier start1;
  PhaseBarrier start2;
};

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){
  auto start1 = runtime->create_phase_barrier(ctx, 1);
  auto start2 = runtime->create_phase_barrier(ctx, 1);
  
  //start1.arrive();
  runtime->advance_phase_barrier(ctx, start1);
  runtime->advance_phase_barrier(ctx, start2);

  test_args args1;
  test_args args2;
  
  args1.start1 = start1;
  args1.start2 = start2;

  args2.start1 = start1;
  args2.start2 = start2;

  for (int i = 0; i < 100; i++) {
    args1.ts = i;
    args2.ts = i;

    TaskLauncher launcher1(TEST1_TASK_ID,
      TaskArgument(&args1,sizeof(args1)));
    
    TaskLauncher launcher2(TEST2_TASK_ID,
      TaskArgument(&args2,sizeof(args2)));

    Future f2 = runtime->execute_task(ctx, launcher2);
    Future f1 = runtime->execute_task(ctx, launcher1);

    f1.get_void_result();
    f2.get_void_result();
  }
}

void test1_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){
  test_args* args = (test_args*)task->args;
  np(args->ts);
  args->start1.wait();
  runtime->advance_phase_barrier(ctx, args->start2);
  //args->start2.arrive();
}

void test2_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){
  test_args* args = (test_args*)task->args;
  np(args->ts);
  args->start2.wait();
  runtime->advance_phase_barrier(ctx, args->start1);
  //args->start1.arrive();
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
    Processor::LOC_PROC, true, false);

  Runtime::register_legion_task<test1_task>(TEST1_TASK_ID,
    Processor::LOC_PROC, true, false);

  Runtime::register_legion_task<test2_task>(TEST2_TASK_ID,
    Processor::LOC_PROC, true, false);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
