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
/// \file legion/dpd.h
/// \authors nickm
// \date Initial file creation: Nov 29, 2016
///

namespace flecsi {
namespace execution {

///
/// FIXME documentation
///
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

  ///
  /// FIXME documentation
  ///
  struct partitioned_unstructured{
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
    size_t size;
    partition_count_map count_map;
  };

  ///
  /// FIXME documentation
  ///
  struct entity_pair{
    size_t e1;
    size_t e2;
  };

  ///
  /// FIXME documentation
  ///
  struct ptr_count{
    ptr_t ptr;
    size_t count;
  };

  ///
  /// FIXME documentation
  ///
  struct offset_count{
    size_t offset;
    size_t count;
  };

  ///
  /// FIXME documentation
  ///
  template<typename T>
  struct entry_value{
    size_t entry;
    T value;
  };

  ///
  /// FIXME documentation
  ///
  struct entry_offset{
    size_t entry;
    size_t offset;
  };

  ///
  /// FIXME documentation
  ///
  struct partition_metadata{
    size_t partition;
    Legion::LogicalRegion lr;
    Legion::IndexPartition ip;
    size_t size;
    size_t reserve;
  };

  ///
  /// FIXME documentation
  ///
  template<class T>
  struct commit_data{
    using spare_map_t = std::multimap<size_t, entry_value<T>>;
    using erase_set_t = std::set<std::pair<size_t, size_t>>;

    size_t partition;
    size_t slot_size;
    size_t num_slots;
    size_t num_indices;
    size_t* indices;
    entry_value<T>* entries;
    spare_map_t spare_map;
    erase_set_t* erase_set = nullptr;
  };

  using connectivity_vec = std::vector<std::vector<size_t>>;

  ///
  /// Default constructor
  ///
  legion_dpd(Legion::Context context,
             Legion::Runtime* runtime)
  : context_(context),
  runtime_(runtime),
  h(runtime, context){}

  ///
  /// FIXME documentation
  ///
  template<class T>
  void create_data(partitioned_unstructured& indices,
                   size_t start_reserve,
                   size_t start_size = 0){
    create_data_(indices, start_reserve, start_size, sizeof(T));
  }

  ///
  /// FIXME documentation
  ///
  template<class T>
  void commit(commit_data<T>& cd){
    commit_(reinterpret_cast<commit_data<char>&>(cd), sizeof(T));
  }

  ///
  /// FIXME documentation
  ///
  void commit_(commit_data<char>& cd, size_t value_size);

  ///
  /// FIXME documentation
  ///
  void create_data_(partitioned_unstructured& indices,
                    size_t start_reserve,
                    size_t start_size,
                    size_t value_size);

  ///
  /// FIXME documentation
  ///
  void create_connectivity(size_t from_dim,
                           partitioned_unstructured& from,
                           size_t to_dim,
                           partitioned_unstructured& to,
                           partitioned_unstructured& raw_connectivity);

  ///
  /// FIXME documentation
  ///
  static void init_connectivity_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context ctx, Legion::Runtime* runtime);

  ///
  /// FIXME documentation
  ///
  static void init_data_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context ctx, Legion::Runtime* runtime);

  ///
  /// FIXME documentation
  ///
  partition_metadata get_partition_metadata(size_t partition);

  ///
  /// FIXME documentation
  ///
  static partition_metadata get_partition_metadata_task(
    const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  ///
  /// FIXME documentation
  ///
  void put_partition_metadata(const partition_metadata& md);

  ///
  /// FIXME documentation
  ///
  static void put_partition_metadata_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  ///
  /// FIXME documentation
  ///
  static partition_metadata commit_data_task(const Legion::Task* task,
    const std::vector<Legion::PhysicalRegion>& regions,
    Legion::Context context, Legion::Runtime* runtime);

  ///
  /// FIXME documentation
  ///
  void dump(size_t from_dim, size_t to_dim);

  ///
  /// FIXME documentation
  ///
  static size_t connectivity_field_id(size_t from_dim, size_t to_dim){
    return 1000 + from_dim * 10 + to_dim;
  }

  ///
  /// FIXME documentation
  ///
  void map_data(size_t partition,
                offset_count*& indices,
                entry_offset*& entries,
                void*& values);
  ///
  /// FIXME documentation
  ///
  void unmap_data();

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
  Legion::PhysicalRegion data_from_pr_;  
  Legion::PhysicalRegion data_pr_;  
  Legion::PhysicalRegion data_values_pr_;  
};

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_dpd_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
