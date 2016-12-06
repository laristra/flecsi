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
    INIT_TID = 10000
  };

  enum fields_ids{
    ENTITY_FID = 10000,
    ENTITY_PAIR_FID,
    PTR_FID,
    PTR_COUNT_FID
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

  using connectivity_vec = std::vector<std::vector<size_t>>;

  legion_dpd(Legion::Context context,
             Legion::Runtime* runtime)
  : context_(context),
  runtime_(runtime),
  h(runtime, context){}

  void create_data(partitioned_unstructured& le);

  void create_connectivity(partitioned_unstructured& from,
                           partitioned_unstructured& to,
                           partitioned_unstructured& raw_connectivity);


  static void init_task(const Legion::Task* task,
                        const std::vector<Legion::PhysicalRegion>& regions,
                        Legion::Context ctx, Legion::Runtime* runtime);

  void dump();

private:
  Legion::Context context_;
  Legion::Runtime* runtime_;
  legion_helper h;

  partitioned_unstructured from_;
  partitioned_unstructured to_;

  Legion::LogicalRegion from_lr_;
  Legion::IndexPartition from_ip_;
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
