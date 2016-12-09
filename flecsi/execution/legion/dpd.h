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
    INIT_DATA_TID
  };

  enum fields_ids{
    ENTITY_FID = 10000,
    ENTITY_PAIR_FID,
    PTR_FID,
    PTR_COUNT_FID,
    ENTRY_FID,
    VALUE_FID,
    DATA_INFO_FID
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

  struct data_info{
    ptr_t ptr;
    size_t size;
    size_t reserve;
  };

  using connectivity_vec = std::vector<std::vector<size_t>>;

  legion_dpd(Legion::Context context,
             Legion::Runtime* runtime)
  : context_(context),
  runtime_(runtime),
  h(runtime, context){}

  template<class T>
  void create_data(partitioned_unstructured& indices,
                   size_t max_entries_per_index,
                   size_t init_reserve){
    create_data_(indices, max_entries_per_index, init_reserve, sizeof(T));
  }  

  void create_data_(partitioned_unstructured& indices,
                    size_t max_entries_per_index,
                    size_t init_reserve,
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
};

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_dpd_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
