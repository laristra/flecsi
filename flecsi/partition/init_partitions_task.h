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

#ifndef init_partitions_task_h
#define init_partitions_task_h

using namespace LegionRuntime::HighLevel;


enum
FieldIDs2 {
  FID_SHARED,
  FID_EXCLUSIVE,
  FID_GHOST,
};

namespace flecsi {
namespace dmp {

struct parts {
  int primary;
  int exclusive;
  int shared;
  int ghost;
};

struct SPMDArgs {
    PhaseBarrier pbarrier_as_master;
    std::vector<PhaseBarrier> masters_pbarriers;
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
init_cells_task(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

Legion::LogicalRegion
shared_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

Legion::LogicalRegion
exclusive_part_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

Legion::LogicalRegion
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

void
ghost_access_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
);

} // namespace dmp
} // namespace flecsi

#endif //init_partitions_task
