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


#ifndef _MPI_MAPPERS_H_
#define _MPI_MAPPERS_H_

#include "legion.h"
#include "task_ids.h"
#include "shim_mapper.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Arrays;

enum {
  MPI_MAPPER_ID       = 1,
};


namespace flecsi
{
namespace mpilegion
{

class MPIMapper : public ShimMapper {
public:
  MPIMapper(Machine machine, HighLevelRuntime *runtime, Processor local);
  virtual ~MPIMapper(void);
  virtual void select_task_options(Task *task);
  typedef TaskSlice DomainSplit;
  virtual void slice_domain(const Task *task, const Domain &domain,
                            std::vector<DomainSplit> &slices);
  virtual bool map_task(Task *task);
  virtual void notify_mapping_failed(const Mappable *mappable);
protected:
  std::vector<Processor> local_cpus;
  std::vector<unsigned> local_next_cpu;
};

void mapper_registration(Machine machine, HighLevelRuntime *rt,
                           const std::set<Processor> &local_procs);


LegionRuntime::Logger::Category log_mapper("mpi_mapper");


MPIMapper::MPIMapper(Machine machine, HighLevelRuntime *rt, Processor local)
  : ShimMapper(machine, rt, rt->get_mapper_runtime(), local)
{

  std::set<Processor> all_procs;
  machine.get_all_processors(all_procs);
  for (std::set<Processor>::const_iterator it = all_procs.begin();
        it != all_procs.end(); it++)
  {
#ifndef SHARED_LOWLEVEL
    // Filter out any processors that aren't local (allow differences only
    //  in the 16 LSBs - should work for 32-bit and 64-bit IDs)
    if (((it->id ^ local_proc.id) >> 16) != 0)
      continue;
#endif
    Processor::Kind k = it->kind();
    if (k == Processor::LOC_PROC)
      local_cpus.push_back(*it);
  }

  //local_next_cpu.resize(MAX_NUM_TASK_ID, 0);
}

MPIMapper::~MPIMapper(void)
{
}


void MPIMapper::select_task_options(Task *task)
{
  // Default values
  task->inline_task = false;
  task->spawn_task = false;
  task->map_locally = false;
  task->profile_task = false;
  task->task_priority = 0;
  task->target_proc = local_proc;
}

void MPIMapper::slice_domain(const Task *task, const Domain &domain,
                              std::vector<DomainSplit> &slices)
{
  // Special cases for startup tasks
  if (task->task_id == CONNECT_MPI_TASK_ID||task->task_id ==HANDOFF_TO_MPI_TASK_ID)
  {
    const int DIM = 2;
    std::vector<Processor> proc_list;
    std::set<Processor> all_procs;
    machine.get_all_processors(all_procs);
    for (std::set<Processor>::const_iterator it = all_procs.begin();
          it != all_procs.end(); it++)
    {
      if (it->kind() != Processor::LOC_PROC)
        continue;
      proc_list.push_back(*it);
    }

    assert(proc_list.size() == (size_t)(domain.get_rect<DIM>().hi[0]+1));
    for (Domain::DomainPointIterator pir(domain); pir; pir++) {
      DomainPoint dp = pir.p;
      Point<DIM> p = dp.get_point<DIM>();
      Processor proc = proc_list[p[0]];
      DomainSplit ds(Domain::from_rect<DIM>(Rect<DIM>(p, p)), proc,
                            false , false );
      slices.push_back(ds);
    }
   return;
  }
   ShimMapper::decompose_index_space(domain, local_cpus, 1/*splitting factor*/, slices);
}

bool MPIMapper::map_task(Task *task)
{
  // Add extra fields onto the await MPI task
 /* if (task->task_id == AWAIT_MPI_TASK_ID)
  {
    for (unsigned idx = 0; idx < task->regions.size(); idx++)
    {
      if ((idx == 0) || (idx == 1))
        task->regions[idx].additional_fields = all_q_fields;
      else if (idx == 2)
        task->regions[idx].additional_fields = all_temp_fields;
      else if (idx == 3)
        task->regions[idx].additional_fields = all_state_fields;
      task->regions[idx].make_persistent = true;
    }
  }
 */
    Memory system_mem = machine_interface.find_memory_kind(task->target_proc, Memory::SYSTEM_MEM);
    assert(system_mem.exists());
    for (unsigned idx = 0; idx < task->regions.size(); idx++)
    {
      // Do a quick check for restricted regions, otherwise put it in system memory
      if (task->regions[idx].restricted)
      {
        assert(task->regions[idx].current_instances.size() == 1);
        task->regions[idx].target_ranking.push_back(
            (task->regions[idx].current_instances.begin())->first);
      }
      else
        task->regions[idx].target_ranking.push_back(system_mem);
      task->regions[idx].virtual_map = false;
      task->regions[idx].enable_WAR_optimization = false;
      task->regions[idx].reduction_list = false;
      task->regions[idx].blocking_factor = task->regions[idx].max_blocking_factor;
    }
  return false;
}


void MPIMapper::notify_mapping_failed(const Mappable *mappable)
{
  switch (mappable->get_mappable_kind())
  {
    case Mappable::TASK_MAPPABLE:
      {
        Task *task = mappable->as_mappable_task();
        int failed_idx = -1;
        for (unsigned idx = 0; idx < task->regions.size(); idx++)
        {
          if (task->regions[idx].mapping_failed)
          {
            failed_idx = idx;
            break;
          }
        }
        log_mapper.error("Failed task mapping for region %d of task %s (%p)\n",
                    failed_idx, task->variants->name, task);
        assert(false);
        break;
      }
    case Mappable::COPY_MAPPABLE:
      {
        Copy *copy = mappable->as_mappable_copy();
        int failed_idx = -1;
        for (unsigned idx = 0; idx < copy->src_requirements.size(); idx++)
        {
          if (copy->src_requirements[idx].mapping_failed)
          {
            failed_idx = idx;
            break;
          }
        }
        for (unsigned idx = 0; idx < copy->dst_requirements.size(); idx++)
        {
          if (copy->dst_requirements[idx].mapping_failed)
          {
            failed_idx = copy->src_requirements.size() + idx;
            break;
          }
        }
        log_mapper.error("Failed copy mapping for region %d of copy (%p)\n",
                    failed_idx, copy);
        assert(false);
        break;
      }
    default:
      assert(false);
  }
}

/*
int RHSFMapper::get_tunable_value(const Task *task, TunableID tid, MappingTagID tag)
{
  switch (tid)
  {
    case TUNABLE_PROCS_PER_NODE:
      {
        // Assume that this is the same everywhere
        assert(local_cpus.size() > 0);
        return local_cpus.size();
      }
    default:
      assert(false);
  }
  return -1;
}
*/


void mapper_registration(Machine machine, HighLevelRuntime *rt,
                          const std::set<Processor> &local_procs)
{
  for (std::set<Processor>::const_iterator it = local_procs.begin();
        it != local_procs.end(); it++)
  {
    rt->replace_default_mapper(
        new MPIMapper(machine, rt, *it), *it);
  }
}

}//end namespace mpilegion

}//end namespace flecsi 
#endif

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

