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

struct SPMDArgs {
    PhaseBarrier pbarrier_as_master;
    std::vector<PhaseBarrier> masters_pbarriers;
};

class TaskWrapper {
public:
	TaskWrapper(
			SPMDArgs* args,
			std::vector<LogicalRegion> lrs_shared,
			std::vector<PhysicalRegion> prs_shared,
			LogicalRegion lregion_ghost,
			size_t ghost_copy_task_id,
			FieldID ghost_copy_fid);
	Future execute_task(
			Legion::Context ctx,
			Legion::HighLevelRuntime *runtime,
			TaskLauncher& launcher,
			bool read_phase,
			bool write_phase);
private:
	void write_prologue(TaskLauncher& launcher);
	void write_epilogue(Legion::Context ctx, Legion::HighLevelRuntime *runtime);
	void read_prologue(Legion::Context ctx, Legion::HighLevelRuntime *runtime);
	SPMDArgs* spmd_args;
	std::vector<LogicalRegion> lregions_neighbors_shared;
	std::vector<PhysicalRegion> pregions_neighbors_shared;
	LogicalRegion lregion_ghost;
	size_t ghost_copy_task_id;
	FieldID ghost_fid;
	bool is_readable;
};

class ArgsSerializer {
public:
    ArgsSerializer() {bit_stream = nullptr; bit_stream_size = 0; free_bit_stream = false;}
    ~ArgsSerializer() {if (free_bit_stream) free(bit_stream);}
    void* getBitStream();
    size_t getBitStreamSize();
    void setBitStream(void* bit_stream);
protected:
    void* bit_stream;
    size_t bit_stream_size;
    bool free_bit_stream;
};

class SPMDArgsSerializer : public ArgsSerializer {
public:
    void archive(SPMDArgs* spmd_args);
    void restore(SPMDArgs* spmd_args);
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
debug_task(
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
check_partitioning_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
ghost_access_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);


void
ghost_check_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);


void
ghost_init_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

void
halo_copy_task(
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
