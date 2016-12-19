/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_init_partitions_task_h
#define flecsi_partition_init_partitions_task_h

///
// \file
// \date Initial file creation: Dec 19, 2016
///

namespace flecsi {
namespace partition {

enum
FieldIDs2 {
  FID_SHARED,
  FID_EXCLUSIVE,
  FID_GHOST,
};

namespace flecsi {
namespace dmp {

struct parts {
  int primary_cells;
  int exclusive_cells;
  int shared_cells;
  int ghost_cells;
  int primary_vertices;
  int exclusive_vertices;
  int shared_vertices;
  int ghost_vertices;
};

struct partition_lr{
  Legion::LogicalRegion cells;
  Legion::LogicalRegion vert;
};

parts 
get_numbers_of_cells_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
initialization_task(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

partition_lr
shared_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

partition_lr
exclusive_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

partition_lr
ghost_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
check_partitioning_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

} // namespace partition
} // namespace flecsi

#endif // flecsi_partition_init_partitions_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
