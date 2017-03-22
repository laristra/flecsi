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
  GET_METADATA_TASK_ID,
  INIT_ENTITY_TASK_ID
};

using partition_vec = vector<vector<size_t>>;

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

struct ed_metadata{
  IndexPartition exclusive_ip;
  IndexPartition shared_ip;
  IndexPartition ghost_ip;
};

const size_t MAX_DATA_SIZE = 1048576;

struct ed_init_args{
  IndexSpace index_space;
  size_t num_partitions;
  size_t num_exclusive;
  size_t num_shared;
  size_t num_ghost;
  size_t data[MAX_DATA_SIZE];
};

class entity_data{
public:
  entity_data(Runtime* runtime,
              Context context,
              size_t num_partitions,
              size_t num_entities,
              const partition_vec& exclusive,
              const partition_vec& shared,
              const partition_vec& ghost)
  : runtime_(runtime), 
  context_(context),
  h(runtime_, context_),
  num_partitions_(num_partitions),
  num_entities_(num_entities),
  index_partitions_(num_partitions){

    index_space_ = h.create_index_space(0, num_entities_ - 1);

    IndexSpace is = h.create_index_space(0, num_partitions_ - 1);
    FieldSpace fs = h.create_field_space();
    FieldAllocator a = h.create_field_allocator(fs);
    a.allocate_field(sizeof(ed_metadata), 0);
    meta_data_ = h.create_logical_region(is, fs);

    Blockify<1> coloring(1);

    IndexPartition ip = 
      runtime_->create_index_partition(context_, is, coloring);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_, meta_data_, ip);

    ArgumentMap arg_map;

    ed_init_args ei;
    ei.num_partitions = num_partitions;
    ei.index_space = index_space_; 

    for(size_t p = 0; p < num_partitions; ++p){

      copy(exclusive[p].begin(), exclusive[p].end(), ei.data);
      size_t n = exclusive[p].size();

      copy(shared[p].begin(), shared[p].end(), &ei.data[n]);
      n += shared[p].size();

      copy(exclusive[p].begin(), exclusive[p].end(), &ei.data[n]);
      n += exclusive[p].size();

      size_t trim = MAX_DATA_SIZE - n;

      ei.num_exclusive = exclusive[p].size();
      ei.num_shared = shared[p].size();
      ei.num_ghost = ghost[p].size();

      arg_map.set_point(h.domain_point(p),
        TaskArgument(&ei, sizeof(ei) - trim * sizeof(size_t)));
    }

    Domain d = h.get_index_space_domain(is);

    IndexLauncher il(INIT_TASK_ID, d, TaskArgument(nullptr, 0), arg_map);

    il.add_region_requirement(
          RegionRequirement(lp, 0/*projection ID*/, 
                            WRITE_DISCARD, EXCLUSIVE, meta_data_));
    il.region_requirements[0].add_field(0);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();

    // ------

    create_field<size_t>(0);

    {
      LogicalRegion lr = get_field(0);
      vector<ed_metadata> mds;
      get_all_metadata(mds);

      vector<FutureMap> fms;

      for(size_t p = 0; p < num_partitions_; ++p){
        ed_metadata& md = mds[p];
        IndexPartition ip = md.exclusive_ip;

        LogicalPartition lp =
          runtime_->get_logical_partition(context_, lr, ip);

        IndexSpace is = runtime_->get_index_subspace(ip, p);

        //Domain d = h.get_index_space_domain(is);
        ArgumentMap arg_map;

        Domain d = h.domain_from_rect(0, num_partitions - 1);

        IndexLauncher il(INIT_ENTITY_TASK_ID, d,
          TaskArgument(nullptr, 0), arg_map);

        il.add_region_requirement(
              RegionRequirement(lp, 0, 
                                WRITE_DISCARD, EXCLUSIVE, lr));
        il.region_requirements[0].add_field(0);

        FutureMap fm = h.execute_index_space(il);
        fms.push_back(fm);
      }

      for(FutureMap& fm : fms){
        fm.wait_all_results();
      }
    }
  }

  template<class T>
  void create_field(size_t id){
    static_assert(std::is_pod<T>::value, "T is not POD");

    assert(field_map_.find(id) == field_map_.end());

    FieldSpace fs = h.create_field_space();

    FieldAllocator a = h.create_field_allocator(fs);
    a.allocate_field(sizeof(T), id);
    
    LogicalRegion lr = h.create_logical_region(index_space_, fs);

    field_map_.emplace(id, lr);
  }

  LogicalRegion get_field(size_t id){
    auto itr = field_map_.find(id);
    assert(itr != field_map_.end());
    return itr->second;
  }

  ed_metadata get_metadata(size_t partition){
    IndexSpace is = meta_data_.get_index_space();
    Domain domain = h.get_index_space_domain(is);

    Blockify<1> coloring(1);

    IndexPartition ip = 
      runtime_->create_index_partition(context_, is, coloring);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_, meta_data_, ip);

    Domain d = h.domain_from_point(partition);

    ArgumentMap arg_map;

    IndexLauncher il(GET_METADATA_TASK_ID, d,
      TaskArgument(nullptr, 0), arg_map);

    il.add_region_requirement(
          RegionRequirement(lp, 0/*projection ID*/, 
                            READ_ONLY, EXCLUSIVE, meta_data_));
    il.region_requirements[0].add_field(0);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
    DomainPoint dp = DomainPoint::from_point<1>(make_point(partition));
    ed_metadata md = fm.get_result<ed_metadata>(dp);
    return md;
  }

  void get_all_metadata(vector<ed_metadata>& mds){
    IndexSpace is = meta_data_.get_index_space();
    Domain domain = h.get_index_space_domain(is);

    Blockify<1> coloring(1);

    IndexPartition ip = 
      runtime_->create_index_partition(context_, is, coloring);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_, meta_data_, ip);

    Domain d = h.get_index_space_domain(is);

    ArgumentMap arg_map;

    IndexLauncher il(GET_METADATA_TASK_ID, d,
      TaskArgument(nullptr, 0), arg_map);

    il.add_region_requirement(
          RegionRequirement(lp, 0/*projection ID*/, 
                            READ_ONLY, EXCLUSIVE, meta_data_));
    il.region_requirements[0].add_field(0);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();

    for(size_t p = 0; p < num_partitions_; ++p){
      ed_metadata md = fm.get_result<ed_metadata>(h.domain_point(p));   
      mds.emplace_back(move(md));
    }
  }

private:
  using field_map_t = unordered_map<size_t, LogicalRegion>;

  Runtime* runtime_;
  Context context_;
  field_map_t field_map_;
  legion_helper h;
  size_t num_partitions_;
  size_t num_entities_;
  IndexSpace index_space_;
  vector<IndexPartition> index_partitions_;
  LogicalRegion meta_data_;
};

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context context, Runtime* runtime){

  size_t num_partitions = 10;
  size_t num_entities = 1000;

  partition_vec exclusive(num_partitions);
  partition_vec shared(num_partitions);
  partition_vec ghost(num_partitions);
  
  for(size_t i = 0; i < num_entities; ++i){
    size_t p = equilikely(0, num_partitions - 1);
    exclusive[p].push_back(i);
  }

  for(size_t p = 0; p < num_partitions; ++p){
    auto& s = exclusive[p];

    for(size_t si : s){
      if(uniform() < 0.1){
        shared[p].push_back(si);
        
        size_t n = equilikely(1, 2);
        
        set<size_t> es;
        es.insert(p);
        
        for(size_t j = 0; j < n; ++j){
          size_t gp;
          
          for(;;){
            gp = equilikely(0, num_partitions - 1);
          
            if(es.find(gp) == es.end()){
              es.insert(gp);
              break;
            }
          }
          
          ghost[gp].push_back(si);
        }
      }
    }
  }

  entity_data ed(runtime, context, num_partitions, num_entities,
    exclusive, shared, ghost);
}

void init_task(const Task* task,
               const std::vector<PhysicalRegion>& regions,
               Context context, Runtime* runtime){
  legion_helper h(runtime, context);

  ed_init_args* args = (ed_init_args*)task->local_args;

  size_t p = task->index_point.point_data[0];

  ed_metadata md;

  size_t start = 0;
  size_t end = args->num_exclusive;

  {
    set<Domain> ds;
    for(size_t i = start; i < end; ++i){
      ds.emplace(h.domain_from_point(args->data[i]));
    }

    MultiDomainColoring mdc;
    mdc[p] = move(ds);

    // ndm - all colors - or just for this partition?
    Domain colors = h.domain_from_rect(0, args->num_partitions - 1);
    //Domain colors = h.domain_from_point(p);

    // ndm - correct?
    md.exclusive_ip = 
      runtime->create_index_partition(context, args->index_space, colors, mdc, true);
  }

  start = end;
  end += args->num_shared;

  {
    set<Domain> ds;
    for(size_t i = start; i < end; ++i){
      ds.emplace(h.domain_from_point(args->data[i]));
    }

    MultiDomainColoring mdc;
    mdc[p] = move(ds);

    // ndm - all colors - or just for this partition?
    //Domain colors = h.domain_from_rect(0, args->num_partitions - 1);
    Domain colors = h.domain_from_point(p);

    // ndm - correct?
    md.shared_ip = 
      runtime->create_index_partition(context, args->index_space, colors, mdc, true);
  }

  start = end;
  end += args->num_ghost;

  {
    set<Domain> ds;
    for(size_t i = start; i < end; ++i){
      ds.emplace(h.domain_from_point(args->data[i]));
    }

    MultiDomainColoring mdc;
    mdc[p] = move(ds);

    // ndm - all colors - or just for this partition?
    Domain colors = h.domain_from_rect(0, args->num_partitions - 1);

    // ndm - correct?
    md.ghost_ip = 
      runtime->create_index_partition(context, args->index_space, colors, mdc, true);
  }

  auto ac = regions[0].get_field_accessor(0).typeify<ed_metadata>();
  ac.write(DomainPoint::from_point<1>(p), md);
}

ed_metadata get_metadata_task(const Task* task,
  const std::vector<PhysicalRegion>& regions,
  Context context, Runtime* runtime){

  legion_helper h(runtime, context);

  size_t p = task->index_point.point_data[0];

  auto ac = regions[0].get_field_accessor(0).typeify<ed_metadata>();
  ed_metadata md = ac.read(DomainPoint::from_point<1>(p));
  h.unmap_region(regions[0]);
  return md;
}

void init_entity_task(const Task* task,
  const std::vector<PhysicalRegion>& regions,
  Context context, Runtime* runtime){

  legion_helper h(runtime, context);

  size_t p = task->index_point.point_data[0];

  np(p);
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
    Processor::LOC_PROC, true/*single*/, false/*index*/);

  Runtime::register_legion_task<init_task>(INIT_TASK_ID,
    Processor::LOC_PROC, false/*single*/, true/*index*/);

  Runtime::register_legion_task<ed_metadata, get_metadata_task>(GET_METADATA_TASK_ID,
    Processor::LOC_PROC, false/*single*/, true/*index*/);

 Runtime::register_legion_task<init_entity_task>(INIT_ENTITY_TASK_ID,
   Processor::LOC_PROC, false/*single*/, true/*index*/);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
