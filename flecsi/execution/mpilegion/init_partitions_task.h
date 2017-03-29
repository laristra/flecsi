/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpilegion_init_partitions_task_h
#define flecsi_execution_mpilegion_init_partitions_task_h

using namespace LegionRuntime::HighLevel;

#include <legion.h>
///
/// \file
/// \date Initial file creation: Dec 19, 2016
///


namespace flecsi {
namespace execution {
namespace sprint{

struct parts {
  int primary_cells;
  int exclusive_cells;
  int shared_cells;
  int ghost_cells;
  int primary_vertices;
  int exclusive_vertices;
  int shared_vertices;
  int ghost_vertices;
  int vertex_conns;
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
copy_legion_to_flecsi_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
size_t_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
double_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
init_raw_conn_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

}  //namespace temporary
} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_mpilegion_init_partitions_task_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
