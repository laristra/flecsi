#include <cinchtest.h>
#include <iostream>
#include <sstream>

#include "flecsi/topology/mesh_topology.h"

#include "legion.h"
#include "legion_config.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

using namespace std;

using namespace Legion;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::HighLevel;

using namespace flecsi;
using namespace topology;

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
  SUBTASK_ID,
};

struct SPMDArgs {
    int my_color;
    PhaseBarrier pbarrier;
};

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{
    const int num_subregions = 2;
    PhaseBarrier one_barrier = runtime->create_phase_barrier(ctx, num_subregions);

    MustEpochLauncher must_epoch_launcher;
    std::vector<SPMDArgs> args(num_subregions);
    for (int i = 0; i < num_subregions; i++)
    {
        args[i].pbarrier = one_barrier;
        args[i].my_color = i;

        TaskLauncher launcher(SUBTASK_ID, TaskArgument(&args[i],sizeof(args[i])));
        DomainPoint point(i);
        must_epoch_launcher.add_single_task(point, launcher);
    }
    FutureMap fm = runtime->execute_must_epoch(ctx, must_epoch_launcher);
    fm.wait_all_results();

    std::cout << "Test completed." << std::endl;

    runtime->destroy_phase_barrier(ctx, one_barrier);

}

void subtask(const Task *task,
                   const std::vector<PhysicalRegion> &regions,
                   Context ctx, HighLevelRuntime *runtime)
{
    const int num_phases = 10;

    assert(task->arglen == sizeof(SPMDArgs));
    SPMDArgs args = *(const SPMDArgs*)task->args;

    for (int i = 0; i < num_phases; i++ ) {
        args.pbarrier.wait();
        std::cout << "Task " << args.my_color << " phase " << i << std::endl;
        args.pbarrier.arrive(1);
        args.pbarrier = runtime->advance_phase_barrier(ctx, args.pbarrier);
    }
}

TEST(legion, test1) {
  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/);
  HighLevelRuntime::register_legion_task<subtask>(SUBTASK_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/);

  int argc = 1;
  char** argv = (char**)malloc(sizeof(char*));
  argv[0] = strdup("-test");

  HighLevelRuntime::start(argc, argv)
}
