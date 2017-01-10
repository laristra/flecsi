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

#ifndef flecsi_execution_legion_dpd_h
#define flecsi_execution_legion_dpd_h

#include <unordered_map>

#include <legion.h>

#include "flecsi/execution/legion/helper.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

///
// \file legion/dpd.h
// \authors nickm
// \date Initial file creation: Nov 29, 2016
///

namespace flecsi {
namespace execution {

class legion_dpd{
public:
  enum task_ids{
    INIT_CONNECTIVITY_TID = 10000,
    INIT_DATA_TID,
    GET_PARTITION_METADATA_TID,
    PUT_PARTITION_METADATA_TID,
    COMMIT_DATA_TID
  };

  using partition_count_map = std::map<size_t, size_t>;

  struct partitioned_unstructured{
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
    size_t size;
    partition_count_map count_map;
  };

  struct entity_pair{
    size_t e1;
    size_t e2;
  };

  struct ptr_count{
    ptr_t ptr;
    size_t count;
  };

  struct offset_count{
    size_t offset;
    size_t count;
  };

  template<typename T>
  struct entry_value{
    size_t entry;
    T value;
  };

  struct entry_offset{
    size_t entry;
    size_t offset;
  };

  struct partition_metadata{
    size_t partition;
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
    size_t size;
    size_t reserve;
  };

  using index_pair = std::pair<size_t, size_t>;

  template<class T>
  struct commit_data{
    using spare_map_t = std::multimap<size_t, entry_value<T>>;
    using erase_set_t = std::set<std::pair<size_t, size_t>>;

    size_t partition;
    size_t slot_size;
    size_t num_slots;
    size_t num_indices;
    index_pair* indices;
    entry_value<T>* entries;
    spare_map_t spare_map;
    erase_set_t* erase_set = nullptr;
  };

  using connectivity_vec = std::vector<std::vector<size_t>>;

  legion_dpd(Legion::Context context,
             Legion::Runtime* runtime)
  : context_(context),
  runtime_(runtime),
  h(runtime, context){}

  template<class T>
  void create_data(partitioned_unstructured& indices,
                   size_t start_reserve){
    create_data_(indices, start_reserve, sizeof(T));
  }

  template<class T>
  void commit(commit_data<T>& cd){
    commit_(reinterpret_cast<commit_data<char>&>(cd), sizeof(T));
  }

  void commit_(commit_data<char>& cd, size_t value_size);

  void create_data_(partitioned_unstructured& indices,
                    size_t start_reserve,
                    size_t value_size);

  void create_connectivity(size_t from_dim,
                           partitioned_unstructured& from,
                           size_t to_dim,
                           partitioned_unstructured& to,
                           partitioned_unstructured& raw_connectivity);


  static void init_connectivity_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context ctx, Legion::Runtime* runtime);

  static void init_data_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context ctx, Legion::Runtime* runtime);

  partition_metadata get_partition_metadata(size_t partition);

  static partition_metadata get_partition_metadata_task(
    const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  void put_partition_metadata(const partition_metadata& md);

  static void put_partition_metadata_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  static partition_metadata commit_data_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  void dump(size_t from_dim, size_t to_dim);

  static size_t connectivity_field_id(size_t from_dim, size_t to_dim){
    return 1000 + from_dim * 10 + to_dim;
  }

private:
  Legion::Context context_;
  Legion::Runtime* runtime_;
  legion_helper h;

  partitioned_unstructured from_;
  partitioned_unstructured to_;

  Legion::LogicalRegion to_lr_;
  Legion::IndexPartition to_ip_;

  Legion::LogicalRegion partition_metadata_lr_;
  Legion::IndexPartition partition_metadata_ip_;  
};

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_dpd_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
