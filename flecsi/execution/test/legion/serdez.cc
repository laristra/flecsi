/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "legion.h"
#include "mpi.h"

#include "cinchtest.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
  INIT_TASK_ID,
  CHECK_TASK_ID,
};

enum FieldIDs {
  FID_VAL,
};

enum SerdezIDs { SERDEZ_ID = 123 };

struct my_vector_t {
  int size;
  int * data;
};

class MySerdezObject
{
public:
  typedef my_vector_t FIELD_TYPE;
  static const size_t MAX_SERIALIZED_SIZE = 4096;

  static size_t serialized_size(const FIELD_TYPE & val) {
    return sizeof(int) + (val.size * sizeof(int));
  }

  // note:  serialize and deserialize could also be implemented using
  //        the Realm::Serialization classes
  static size_t serialize(const FIELD_TYPE & val, void * buffer) {
    char * buf = (char *)buffer;
    memcpy(buf, &val.size, sizeof(int));
    buf += sizeof(int);
    int s = val.size * sizeof(int);
    memcpy(buf, val.data, s);
    return s + sizeof(int);
  }

  static size_t deserialize(FIELD_TYPE & val, const void * buffer) {
    const char * buf = (char *)buffer;
    memcpy(&val.size, buf, sizeof(int));
    buf += sizeof(int);
    int s = val.size * sizeof(int);
    val.data = new int[val.size];
    memcpy(val.data, buf, s);
    return s + sizeof(int);
  }

  static void destroy(FIELD_TYPE & val) {
    delete val.data;
  }
};

struct config_t {
  int num_elmts, num_ghosts, num_colors;
};

int num_cpus = 0;

void
top_level_task(const Task * task,
  const std::vector<PhysicalRegion> & regions,
  Context ctx,
  HighLevelRuntime * runtime) {
  const int num_elmts = 64;
  const int num_ghosts = 2;
  const int num_colors = num_cpus;

  Rect<1> rect(Point<1>(0), Point<1>(num_elmts - 1));
  IndexSpace is = runtime->create_index_space(ctx, Domain::from_rect<1>(rect));
  FieldSpace fs = runtime->create_field_space(ctx);
  {
    FieldAllocator allocator = runtime->create_field_allocator(ctx, fs);
    allocator.allocate_field(sizeof(my_vector_t), FID_VAL, SERDEZ_ID);
  }
  LogicalRegion lr = runtime->create_logical_region(ctx, is, fs);

  printf("Field size = %d\n", num_elmts);
  printf("Number of partitions = %d\n", num_colors);

  Rect<1> color_bounds(Point<1>(0), Point<1>(num_colors - 1));
  Domain color_domain = Domain::from_rect<1>(color_bounds);
  DomainColoring coloring, coloring2;
  for(int color = 0; color < num_colors; color++) {
    // disjoint coloring
    int start = color * (rect.volume() / num_colors);
    int stop = (color + 1) * (rect.volume() / num_colors);
    Rect<1> sub_rect(make_point(start), make_point(stop - 1));
    coloring[color] = Domain::from_rect<1>(sub_rect);

    // overlapping coloring (with ghosts)
    if(color > 0)
      start -= num_ghosts;
    if(color < num_colors - 1)
      stop += num_ghosts;
    Rect<1> sub_rect2(make_point(start), make_point(stop - 1));
    coloring2[color] = Domain::from_rect<1>(sub_rect2);
  }

  IndexPartition ip = runtime->create_index_partition(
    ctx, lr.get_index_space(), color_domain, coloring, true);
  LogicalPartition lp = runtime->get_logical_partition(ctx, lr, ip);
  IndexPartition ip2 = runtime->create_index_partition(
    ctx, lr.get_index_space(), color_domain, coloring2, false);
  LogicalPartition lp2 = runtime->get_logical_partition(ctx, lr, ip2);

  config_t config{num_elmts, num_ghosts, num_colors};
  ArgumentMap arg_map;

  // initialize field using disjoint partition
  IndexLauncher init_launcher(INIT_TASK_ID, color_domain,
    TaskArgument(&config, sizeof(config_t)), arg_map);
  init_launcher.add_region_requirement(
    RegionRequirement(lp, 0, WRITE_DISCARD, EXCLUSIVE, lr));
  init_launcher.add_field(0, FID_VAL);
  runtime->execute_index_space(ctx, init_launcher);

  // check field using overlap partition
  // (verifies that ghosts got passed correctly)
  IndexLauncher check_launcher(CHECK_TASK_ID, color_domain,
    TaskArgument(&config, sizeof(config_t)), arg_map);
  check_launcher.add_region_requirement(
    RegionRequirement(lp2, 0, READ_ONLY, EXCLUSIVE, lr));
  check_launcher.add_field(0, FID_VAL);
  auto fm = runtime->execute_index_space(ctx, check_launcher);
  fm.wait_all_results();

  // clean up our region, index space, and field space
  runtime->destroy_logical_region(ctx, lr);
  runtime->destroy_field_space(ctx, fs);
  runtime->destroy_index_space(ctx, is);
}

void
init_task(const Task * task,
  const std::vector<PhysicalRegion> & regions,
  Context ctx,
  HighLevelRuntime * runtime) {
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  config_t config = *((config_t *)task->args);
  int num_elmts = config.num_elmts;
  int num_ghosts = config.num_ghosts;
  int num_colors = config.num_colors;

  FieldID fid = *(task->regions[0].privilege_fields.begin());

  RegionAccessor<AccessorType::Generic, my_vector_t> acc =
    regions[0].get_field_accessor(fid).typeify<my_vector_t>();
  Domain dom = runtime->get_index_space_domain(
    ctx, task->regions[0].region.get_index_space());
  Rect<1> rect = dom.get_rect<1>();

  const int my_color = runtime->find_local_MPI_rank();
  int count = my_color * (num_elmts / num_colors);
  for(GenericPointInRectIterator<1> pir(rect); pir; pir++) {
    my_vector_t vec;
    int size = (count % 2 ? 4 : 3);
    vec.size = size;
    vec.data = new int[size];
    for(int i = 0; i < size; i++) {
      int id = count * 100 + i;
      vec.data[i] = id;
    }
    acc.write(DomainPoint::from_point<1>(pir.p), vec);
    ++count;
  }
} // init_task

void
check_task(const Task * task,
  const std::vector<PhysicalRegion> & regions,
  Context ctx,
  HighLevelRuntime * runtime) {
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  config_t config = *((config_t *)task->args);
  int num_elmts = config.num_elmts;
  int num_ghosts = config.num_ghosts;
  int num_colors = config.num_colors;

  FieldID fid = *(task->regions[0].privilege_fields.begin());

  RegionAccessor<AccessorType::Generic, my_vector_t> acc =
    regions[0].get_field_accessor(fid).typeify<my_vector_t>();
  Domain dom = runtime->get_index_space_domain(
    ctx, task->regions[0].region.get_index_space());
  Rect<1> rect = dom.get_rect<1>();

  const int my_color = runtime->find_local_MPI_rank();
  int exp_start = my_color * (num_elmts / num_colors);
  if(my_color > 0)
    exp_start -= num_ghosts;
  int exp_stop = (my_color + 1) * (num_elmts / num_colors);
  if(my_color < num_colors - 1)
    exp_stop += num_ghosts;
  int count = exp_start;
  for(GenericPointInRectIterator<1> pir(rect); pir; pir++) {
    my_vector_t vec = acc.read(DomainPoint::from_point<1>(pir.p));
    int exp_size = (count % 2 ? 4 : 3);
    int size = vec.size;
    ASSERT_EQ(size, exp_size);
    // printf("%d: %d\n", count, size);
    for(int i = 0; i < size; i++) {
      int exp_id = count * 100 + i;
      int id = vec.data[i];
      // printf("%d: %d == %d\n", count, id, exp_id);
      ASSERT_EQ(id, exp_id);
    }
    ++count;
  }
  ASSERT_EQ(count, exp_stop);
}

int
driver_initialization(int argc, char ** argv) {
  // register tasks
  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
    Processor::LOC_PROC, true /*single*/, false /*index*/, AUTO_GENERATE_ID,
    TaskConfigOptions(false /*leaf task*/), "top_level_task");
  HighLevelRuntime::register_legion_task<init_task>(INIT_TASK_ID,
    Processor::LOC_PROC, true /*single*/, true /*index*/, AUTO_GENERATE_ID,
    TaskConfigOptions(true /*leaf task*/), "init_task");
  HighLevelRuntime::register_legion_task<check_task>(CHECK_TASK_ID,
    Processor::LOC_PROC, true /*single*/, true /*index*/, AUTO_GENERATE_ID,
    TaskConfigOptions(true /*leaf task*/), "check_task");

  // register custom serdez
  HighLevelRuntime::register_custom_serdez_op<MySerdezObject>(SERDEZ_ID);

  // set up MPI interop
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_cpus);
  HighLevelRuntime::configure_MPI_interoperability(rank);

  return HighLevelRuntime::start(argc, argv);
}
