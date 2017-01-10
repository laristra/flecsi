#include <cinchtest.h>
#include <iostream>
#include <sstream>
#include <map>
#include <cereal/archives/binary.hpp>

#include "flecsi/execution/legion/dpd.h"
#include "flecsi/execution/task_ids.h"

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

const size_t NUM_CELLS = 16;
const size_t NUM_PARTITIONS = 4;

template<typename T>
struct entry_value__
{
  entry_value__(size_t entry)
  : entry(entry){}

  entry_value__(size_t entry, T value)
  : entry(entry),
  value(value){}

  entry_value__(){}

  size_t entry;
  T value;
}; // struct entry_value__

using index_pair_ = std::pair<size_t, size_t>;

class sparse_data{
public:
  sparse_data(legion_dpd& dpd,
              size_t partition,
              size_t num_indices,
              size_t num_entries)
  : dpd_(dpd),
  partition_(partition),
  num_indices_(num_indices),
  num_entries_(num_entries){}

  legion_dpd& dpd(){
    return dpd_;
  }

  size_t partition() const{
    return partition_;
  }

  size_t num_indices() const{
    return num_indices_;
  }

  size_t num_entries() const{
    return num_entries_;
  }

private:
  legion_dpd& dpd_;
  size_t num_entries_;  
  size_t num_indices_;  
  size_t partition_;  
};

template<typename T>
class sparse_mutator{
public:
  using entry_value_t = entry_value__<T>;

  sparse_mutator(sparse_data& data,
                 size_t num_slots)
  : data_(data),
  num_slots_(num_slots),
  num_indices_(data_.num_indices()),
  num_entries_(data_.num_entries()),
  indices_(new index_pair_[num_indices_]),
  entries_(new entry_value_t[num_indices_ * num_slots_]),
  erase_set_(nullptr){
    for(size_t i = 0; i < num_indices_; ++i){
      indices_[i].first = 0;
      indices_[i].second = 0;
    }
  }

  ~sparse_mutator()
  {
    commit();
  } // ~sparse_mutator_t

  void commit(){
    if(!indices_) {
      return;
    } // if

    legion_dpd& dpd = data_.dpd();

    legion_dpd::commit_data<T> cd;
    cd.partition = data_.partition();
    cd.slot_size = num_slots_;
    cd.num_slots = num_indices_ * num_slots_;
    cd.num_indices = num_indices_;
    cd.indices = indices_;
    cd.entries = (legion_dpd::entry_value<T>*)entries_;

    dpd.commit(cd);

    delete[] indices_;
    indices_ = nullptr;

    delete[] entries_;
    entries_ = nullptr;
  }

  T &
  operator () (
    size_t index,
    size_t entry
  )
  {
    assert(indices_ && "sparse mutator has alread been committed");
    assert(index < num_indices_ && entry < num_entries_);

    index_pair_ & ip = indices_[index];
    
    size_t n = ip.second - ip.first;
    
    if(n >= num_slots_) {
      return spare_map_.emplace(index, entry_value_t(entry))->second.value;
    } // if

    entry_value_t * start = entries_ + index * num_slots_;     
    entry_value_t * end = start + n;

    entry_value_t * itr = 
      std::lower_bound(start, end, entry_value_t(entry),
        [](const auto & k1, const auto & k2) -> bool{
          return k1.entry < k2.entry;
        });

    while(end != itr) {
      *(end) = *(end - 1);
      --end;
    } // while

    itr->entry = entry;

    ++ip.second;

    return itr->value;
  } // operator ()

  void
  erase(
    size_t index,
    size_t entry
  )
  {
    assert(indices_ && "sparse mutator has alread been committed");
    assert(index < num_indices_ && entry < num_entries_);

    if(!erase_set_){
      erase_set_ = new erase_set_t;
    }

    erase_set_->emplace(std::make_pair(index, entry));
  }

private:
  using spare_map_t = std::multimap<size_t, entry_value__<T>>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;

  sparse_data& data_;
  size_t num_slots_;
  size_t num_indices_;
  size_t num_entries_;
  index_pair_ * indices_;
  entry_value__<T> * entries_;
  spare_map_t spare_map_;
  erase_set_t * erase_set_;
};

void top_level_task(const Task* task,
                    const std::vector<PhysicalRegion>& regions,
                    Context ctx, Runtime* runtime){

  field_ids_t & fid_t = field_ids_t::instance();

  legion_helper h(runtime, ctx);

  partition_vec cells(NUM_CELLS);

  size_t partition_size = NUM_CELLS / NUM_PARTITIONS;

  for(size_t c = 0; c < NUM_CELLS; ++c){
    size_t p = c / partition_size;

    cells[c] = p;
  }

  legion_dpd::partitioned_unstructured cells_part;

  {
    cells_part.size = NUM_CELLS;

    IndexSpace is = h.create_index_space(cells_part.size);

    IndexAllocator ia = runtime->create_index_allocator(ctx, is);
    
    Coloring coloring;

    for(size_t c = 0; c < cells.size(); ++c){
      size_t p = cells[c]; 
      ++cells_part.count_map[p];
    }

    for(auto& itr : cells_part.count_map){
      size_t p = itr.first;
      int count = itr.second;
      ptr_t start = ia.alloc(count);

      for(int i = 0; i < count; ++i){
        coloring[p].points.insert(start + i);
      }
    }

    FieldSpace fs = h.create_field_space();

    FieldAllocator fa = h.create_field_allocator(fs);

    fa.allocate_field(sizeof(entity_id), fid_t.fid_entity);
    
    fa.allocate_field(sizeof(legion_dpd::ptr_count),
                      fid_t.fid_offset_count);

    cells_part.lr = h.create_logical_region(is, fs);

    RegionRequirement rr(cells_part.lr, WRITE_DISCARD, 
      EXCLUSIVE, cells_part.lr);
    
    rr.add_field(fid_t.fid_entity);
    InlineLauncher il(rr);

    PhysicalRegion pr = runtime->map_region(ctx, il);

    pr.wait_until_valid();

    auto ac = 
      pr.get_field_accessor(fid_t.fid_entity).typeify<entity_id>();

    IndexIterator itr(runtime, ctx, is);
    ptr_t ptr = itr.next();

    for(size_t c = 0; c < cells.size(); ++c){
      ac.write(ptr, c);
      ++ptr;
    }

    cells_part.ip = runtime->create_index_partition(ctx, is, coloring, true);

    runtime->unmap_region(ctx, pr);
  }

  legion_dpd dpd(ctx, runtime);
  dpd.create_data<double>(cells_part, 1024);

  size_t partition = 0;
  size_t num_indices = 4;
  size_t num_entries = 100;

  sparse_data data(dpd, partition, num_indices, num_entries);

  {

    sparse_mutator<double> m(data, 5);

    m(0, 2) = 1.2;
    m(0, 1) = 3.1;

  }

  {

    sparse_mutator<double> m(data, 5);

    m(0, 9) = 9.9;
    m(0, 5) = 5.5;
  }

  {

    sparse_mutator<double> m(data, 5);

    m(0, 1) = 1.1;
    m(0, 0) = 0.1;
  }
}

TEST(legion, test1) {
  Runtime::set_top_level_task_id(TOP_LEVEL_TID);
  
  Runtime::register_legion_task<top_level_task>(TOP_LEVEL_TID,
    Processor::LOC_PROC, true, false);

  Runtime::register_legion_task<legion_dpd::init_data_task>(
    legion_dpd::INIT_DATA_TID,
    Processor::LOC_PROC, false, true);

  Runtime::register_legion_task<legion_dpd::partition_metadata,
    legion_dpd::commit_data_task>(legion_dpd::COMMIT_DATA_TID,
    Processor::LOC_PROC, false, true);

  Runtime::register_legion_task<legion_dpd::partition_metadata,
    legion_dpd::get_partition_metadata_task>(
    legion_dpd::GET_PARTITION_METADATA_TID,
    Processor::LOC_PROC, false, true);

  Runtime::register_legion_task<legion_dpd::put_partition_metadata_task>(
    legion_dpd::PUT_PARTITION_METADATA_TID,
    Processor::LOC_PROC, false, true);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  Runtime::start(argc, argv);
}
