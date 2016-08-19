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

#include <legion.h>
#include <shim_mapper.h>

#include "flecsi/execution/mpilegion/task_ids.h"

/*!
* \file mpilegion/mapper.h
* \authors demeshko
* \date Initial file creation: Jul 2016
*
* An implementation of Legion mapper that is nesessary for managing 
* ownership switching between MPI and Legion runtimes
*/

enum {
  MPI_MAPPER_ID = 1,
};


namespace flecsi{
namespace execution{

class mpi_mapper_t : public LegionRuntime::HighLevel::ShimMapper {

  public:
  mpi_mapper_t(LegionRuntime::HighLevel::Machine machine, 
            LegionRuntime::HighLevel::HighLevelRuntime *runtime, 
            LegionRuntime::HighLevel::Processor local);
  virtual ~mpi_mapper_t(void);
  virtual void select_task_options(ShimMapper::Task *legiontask);
  typedef TaskSlice DomainSplit;
  virtual void slice_domain(const Legion::Task *legiontask, 
                            const LegionRuntime::HighLevel::Domain &domain,
                            std::vector<DomainSplit> &slices);
  virtual bool map_task(ShimMapper::Task *legiontask);
  virtual void notify_mapping_failed(const Mappable *mappable);

  protected:
  std::vector<LegionRuntime::HighLevel::Processor> local_cpus;
  std::vector<unsigned> local_next_cpu;
};

/*--------------------------------------------------------------------------*/

inline 
void 
mapper_registration(
   LegionRuntime::HighLevel::Machine machine, 
   LegionRuntime::HighLevel::HighLevelRuntime *rt,
   const std::set<LegionRuntime::HighLevel::Processor> &local_procs);


//LegionRuntime::Logger::Category log_mapper("mpi_mapper");

/*--------------------------------------------------------------------------*/

inline 
mpi_mapper_t::mpi_mapper_t(
  LegionRuntime::HighLevel::Machine machine, 
  LegionRuntime::HighLevel::HighLevelRuntime *rt, 
  LegionRuntime::HighLevel::Processor local
)
  : ShimMapper(machine, rt, rt->get_mapper_runtime(), local)
{
  std::set<LegionRuntime::HighLevel::Processor> all_procs;
  machine.get_all_processors(all_procs);
  for (std::set<LegionRuntime::HighLevel::Processor>::const_iterator 
        it = all_procs.begin();
        it != all_procs.end();
         it++)
  {
#ifndef SHARED_LOWLEVEL
    // Filter out any processors that aren't local (allow differences only
    //  in the 16 LSBs - should work for 32-bit and 64-bit IDs)
    if (((it->id ^ local_proc.id) >> 16) != 0)
      continue;
#endif
    LegionRuntime::HighLevel::Processor::Kind k = it->kind();
    if (k == LegionRuntime::HighLevel::Processor::LOC_PROC)
      local_cpus.push_back(*it);
  }//end for

  //local_next_cpu.resize(MAX_NUM_TASK_ID, 0);
}//mpi_mapper_t

/*--------------------------------------------------------------------------*/

inline 
mpi_mapper_t::~mpi_mapper_t(void)
{
}

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_mapper_t::select_task_options(
   ShimMapper::Task *legiontask
)
{
  // Default values
  legiontask->inline_task = false;
  legiontask->spawn_task = false;
  legiontask->map_locally = false;
  legiontask->profile_task = false;
  legiontask->task_priority = 0;
  legiontask->target_proc = local_proc;
}//select_task_options

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_mapper_t::slice_domain(
  const Legion::Task *legiontask, 
  const LegionRuntime::HighLevel::Domain &domain,
  std::vector<DomainSplit> &slices
)
{
  // Special cases for startup tasks
  if (legiontask->task_id == CONNECT_MPI_TASK_ID
      ||legiontask->task_id ==HANDOFF_TO_MPI_TASK_ID
      || legiontask->task_id ==WAIT_ON_MPI_TASK_ID)
  {
    const int DIM = 2;
    std::vector<LegionRuntime::HighLevel::Processor> proc_list;
    std::set<LegionRuntime::HighLevel::Processor> all_procs;
    machine.get_all_processors(all_procs);
    for (std::set<LegionRuntime::HighLevel::Processor>::const_iterator 
          it = all_procs.begin();
          it != all_procs.end(); 
          it++)
    {
      if (it->kind() != LegionRuntime::HighLevel::Processor::LOC_PROC)
        continue;
      proc_list.push_back(*it);
    }//end for

    assert(proc_list.size() == (size_t)(domain.get_rect<DIM>().hi[0]+1));
    for (LegionRuntime::HighLevel::Domain::DomainPointIterator pir(domain);
          pir; 
          pir++)
    {
      Legion::DomainPoint dp = pir.p;
      LegionRuntime::Arrays::Point<DIM> p = dp.get_point<DIM>();
      LegionRuntime::HighLevel::Processor proc = proc_list[p[0]];
      DomainSplit ds(LegionRuntime::HighLevel::Domain::from_rect<DIM>(
                       Rect<DIM>(p, p)), proc, false , false);
      slices.push_back(ds);
    }//end for
   return;
  }//end if
   ShimMapper::decompose_index_space(domain, local_cpus,
                                  1/*splitting factor*/, slices);
}//slice_domain

/*--------------------------------------------------------------------------*/

inline 
bool 
mpi_mapper_t::map_task(
 ShimMapper::Task *legiontask
)
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
    LegionRuntime::HighLevel::Memory system_mem =
       machine_interface.find_memory_kind(legiontask->target_proc, 
       LegionRuntime::HighLevel::Memory::SYSTEM_MEM);

    assert(system_mem.exists());

    for (unsigned idx = 0; idx < legiontask->regions.size(); idx++)
    {
      // Do a quick check for restricted regions, otherwise
      // put it in system memory
      if (legiontask->regions[idx].restricted)
      {
        assert(legiontask->regions[idx].current_instances.size() == 1);
        legiontask->regions[idx].target_ranking.push_back(
            (legiontask->regions[idx].current_instances.begin())->first);
      }
      else
        legiontask->regions[idx].target_ranking.push_back(system_mem);

        legiontask->regions[idx].virtual_map = false;
        legiontask->regions[idx].enable_WAR_optimization = false;
        legiontask->regions[idx].reduction_list = false;
        legiontask->regions[idx].blocking_factor = 
                           legiontask->regions[idx].max_blocking_factor;
    }//end for
  return false;
}//map_task

/*--------------------------------------------------------------------------*/

inline 
void
mpi_mapper_t::notify_mapping_failed(
const Mappable *mappable
)
{
  switch (mappable->get_mappable_kind())
  {
    case Mappable::TASK_MAPPABLE:
      {
        ShimMapper::Task *legiontask = mappable->as_mappable_task();
        int failed_idx = -1;
        for (unsigned idx = 0; idx < legiontask->regions.size(); idx++)
        {
          if (legiontask->regions[idx].mapping_failed)
          {
            failed_idx = idx;
            break;
          }//end if
        }//end for
        //log_mapper.error("Failed task mapping for region %d of task %s (%p)\n",
         //           failed_idx, legiontask->variants->name, legiontask);
        assert(false);
        break;
      }//TASK_MAPPABLE
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
          }//end if
        }//end for
        for (unsigned idx = 0; idx < copy->dst_requirements.size(); idx++)
        {
          if (copy->dst_requirements[idx].mapping_failed)
          {
            failed_idx = copy->src_requirements.size() + idx;
            break;
          }//end if
        }//end for
        //log_mapper.error("Failed copy mapping for region %d of copy (%p)\n",
          //          failed_idx, copy);
        assert(false);
        break;
      }//COPY_MAPPABLE
    default:
      assert(false);
  }//end switch
}//notify_mapping_failed

/*--------------------------------------------------------------------------*/

inline 
void 
mapper_registration(
  LegionRuntime::HighLevel::Machine machine, 
  LegionRuntime::HighLevel::HighLevelRuntime *rt,
  const std::set<LegionRuntime::HighLevel::Processor> &local_procs
)
{
  for (std::set<LegionRuntime::HighLevel::Processor>::const_iterator 
        it = local_procs.begin();
        it != local_procs.end(); it++)
  {
    rt->replace_default_mapper(
        new mpi_mapper_t(machine, rt, *it), *it);
  }//end for
}//mapper_registration

}//end namespace execution
}//end namespace flecsi 
#endif

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

