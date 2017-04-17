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

#ifndef flecsi_execution_mpilegion_mapper_h
#define flecsi_execution_mpilegion_mapper_h

#include <legion.h>
#include <legion_mapping.h>
#include <default_mapper.h>

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/legion_tasks.h"

/// mapper ID
enum {
  MPI_MAPPER_ID       = 1,
};

///mapper tag's IDs
enum {
  // Use the first 8 bits for storing the rhsf index
  MAPPER_FORCE_RANK_MATCH = 0x00001000,
  MAPPER_SUBRANK_LAUNCH   = 0x00080000,
  MAPPER_ALL_PROC         = 0x00020000
};

namespace flecsi {
namespace execution {

///
/// STLComparator struct compare two Points with dimension=DIM
///
template <
  unsigned DIM
>
struct 
STLComparator 
{
  bool operator()(const LegionRuntime::Arrays::Point<DIM>& a,
    const LegionRuntime::Arrays::Point<DIM>& b) const
  {
    for(unsigned i = 0; i < DIM; i++)  {
      int d = a.x[i] - b.x[i];
      if (d < 0) return true;
      if (d > 0) return false;
    }
     return false;
  }
};

///
/// \class mpi_mapper_t mapper.h
/// \brief  mpi_mapper_t - is a custom mapper that handles mpi-legion
///                      interoperability in FLeCSI
///
class mpi_mapper_t : public Legion::Mapping::DefaultMapper 
{
  public:

  ///
  /// Contructor. Currently supports only LOC_PROC and TOC_PROC
  ///
  mpi_mapper_t(
    LegionRuntime::HighLevel::Machine machine,
    Legion::Runtime *_runtime,
    LegionRuntime::HighLevel::Processor local
  )
  :
    Legion::Mapping::DefaultMapper(
      _runtime->get_mapper_runtime(),
      machine,
      local,
      "default"
    ),
    machine(machine)
  {
    using legion_machine=LegionRuntime::HighLevel::Machine;
    using legion_proc=LegionRuntime::HighLevel::Processor;
    
    legion_machine::ProcessorQuery pq = 
        legion_machine::ProcessorQuery(machine).same_address_space_as(local);   
    for(legion_machine::ProcessorQuery::iterator pqi = pq.begin();
        pqi != pq.end();
        ++pqi)
    {   
      legion_proc p = *pqi;
      if(p.kind() == legion_proc::LOC_PROC)
      	local_cpus.push_back(p);
      else if(p.kind() == legion_proc::TOC_PROC)
        local_gpus.push_back(p);
      else
        continue;
      
      std::map<Realm::Memory::Kind, Realm::Memory>& mem_map = proc_mem_map[p];
      
      legion_machine::MemoryQuery mq = 
          legion_machine::MemoryQuery(machine).has_affinity_to(p);
      for(legion_machine::MemoryQuery::iterator mqi = mq.begin();
          mqi != mq.end();
          ++mqi)
      {           
      	Realm::Memory m = *mqi;
        mem_map[m.kind()] = m;
        
        if(m.kind() == Realm::Memory::SYSTEM_MEM)
          local_sysmem = m;
      }//end for
    }//end for
    
    std::cout << "Mapper constuctor: local=" << local << " cpus=" <<
        local_cpus.size() << " gpus=" << local_gpus.size() <<
        " sysmem=" << local_sysmem<<std::endl;
  }// end mpi_mapper_t
  
  ///
  /// Destructor
  ///
  virtual ~mpi_mapper_t(){};
  
  ///
  ///  The slice_task call is used by the runtime
  ///  to query the mapper about the best way to distribute
  ///  the points in an index space task launch throughout
  ///  the machine.
  ///  To ensure that legion tasks are executed in the same memory space
  ///  as MPI tasks, one need to specify tag = MAPPER_FORCE_RANK_MATCH
  ///  for the index launch int the code.
  ///  By default, slice-task will perform the didtribution the same way 
  ///  it is done in the DefaultMapper. 
  ///
  virtual
  void
  slice_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      const Legion::Mapping::Mapper::SliceTaskInput& input,
      Legion::Mapping::Mapper::SliceTaskOutput& output
             )
  {
    using legion_proc=LegionRuntime::HighLevel::Processor;

    context_t & context_ = context_t::instance();

    // tag-based decisions here
    if(context_.task_id(__flecsi_internal_task_key(handoff_to_mpi_task, loc)) ||
      context_.task_id(__flecsi_internal_task_key(wait_on_mpi_task, loc)) ||
      (task.tag & MAPPER_FORCE_RANK_MATCH) != 0) {

      // expect a 1-D index domain
      assert(input.domain.get_dim() == 1);
      LegionRuntime::Arrays::Rect<1> r = input.domain.get_rect<1>();
      
      // each point in the domain goes to CPUs assigned to that rank
      output.slices.resize(r.volume());
      size_t idx = 0;
      
      using legion_machine=LegionRuntime::HighLevel::Machine;
      using legion_proc=LegionRuntime::HighLevel::Processor;
      
      // get list of all processors and make sure the count matches
      legion_machine::ProcessorQuery pq =
          legion_machine::ProcessorQuery(machine).only_kind(
              legion_proc::LOC_PROC);
      std::vector<legion_proc> all_procs(pq.begin(), pq.end());
      assert((r.lo[0] == 0) && (r.hi[0] == (int)(all_procs.size() - 1)));
      
      for(LegionRuntime::Arrays::GenericPointInRectIterator<1> pir(r);
          pir; ++pir, ++idx) {
      	output.slices[idx].domain =Legion::Domain::from_rect<1>(
            LegionRuntime::Arrays::Rect<1>(pir.p, pir.p));
        output.slices[idx].proc = all_procs[idx];
      }//end for
      return;
    }//endi if MAPPER_FORCE_RANK_MATCH

/*  	if((task.tag & MAPPER_SUBRANK_LAUNCH) != 0) {
    	// expect a 1-D index domain
    	assert(input.domain.get_dim() == 1);

    	// send the whole domain to our local processor
    	output.slices.resize(1);
    	output.slices[0].domain = input.domain;
    	output.slices[0].proc = task.target_proc;
    	return;
  	} //end if MAPPER_SUBRANK_LAUNCH
*/

  	// task-specific cases below
    if((task.tag & MAPPER_ALL_PROC) != 0)
    {
      // expect a 2-D index domain - first component is the processor index,
      //  and second is the MPI rank
      assert(input.domain.get_dim() == 2);
      LegionRuntime::Arrays::Rect<2> r = input.domain.get_rect<2>();
      
      using legion_machine=LegionRuntime::HighLevel::Machine;
      using legion_proc=LegionRuntime::HighLevel::Processor;
      
      // get list of all processors and make sure the count matches
      legion_machine::ProcessorQuery pq =
          legion_machine::ProcessorQuery(machine).only_kind(
              legion_proc::LOC_PROC);
      std::vector<legion_proc> all_procs(pq.begin(), pq.end());
      assert((r.lo[0] == 0) && (r.hi[0] == (int)(all_procs.size() - 1)));
      
      // now create a slice for each processor, allowing it to try to connect to
      //  each MPI rank
      output.slices.resize(all_procs.size());
      for(size_t i = 0; i < all_procs.size(); i++) {
      	LegionRuntime::Arrays::Rect<2> subrect = r;
      	subrect.lo.x[0] = subrect.hi.x[0] = i;
      	output.slices[i].domain = Legion::Domain::from_rect<2>(subrect);
      	output.slices[i].proc = all_procs[i];
      }//end for
      return;
    } //end if task.task_id=
    
    //TOFIX
    /*
      else if( task.task_id ==update_mappers_task_id)
      {
      // expect a 1-D index domain - each point goes to the corresponding node
      assert(input.domain.get_dim() == 1);
      Rect<1> r = input.domain.get_rect<1>();
      
      // go through all the CPU processors and find a representative for each
      //  node (i.e. address space)
      std::map<int, legion_proc> targets;
      
      using legion_machine=LegionRuntime::HighLevel::Machine;
      legion_machine::ProcessorQuery pq =
      legion_machine::ProcessorQuery(machine).only_kind(
      legion_proc::LOC_PROC);

    	for(legion_machine::ProcessorQuery::iterator it = pq.begin();
      	it != pq.end(); ++it)
    	{
      	legion_proc p = *it;
      	int a = p.address_space();
      	if(targets.count(a) == 0)
       		targets[a] = p;
    	}//end for

    	output.slices.resize(r.volume());
    	for(int a = r.lo[0]; a <= r.hi[0]; a++) {
       	assert(targets.count(a) > 0);
       	output.slices[a].domain = Realm::Domain::from_rect<1>(Rect<1>(a, a));
       	output.slices[a].proc = targets[a];
    	}//end for
    	return;
  	} //end else if task.task_id=
*/
    else{
      DefaultMapper::default_slice_task(task, local_cpus, remote_cpus,
                                        input, output, cpu_slices_cache);
    }//end else
  } //end slice_task
  
 protected:
  
  std::map<LegionRuntime::Arrays::Point<1>,
           std::vector<LegionRuntime::HighLevel::Processor>, 
           STLComparator<1> > rank_cpus, rank_gpus;
  std::map<LegionRuntime::HighLevel::Processor,
           std::map<Realm::Memory::Kind, Realm::Memory> > proc_mem_map;
  Realm::Memory local_sysmem;
  Realm::Machine machine;
};

///
/// mapper_registration is used to replace DefaultMapper with mpi_mapper_t in
/// FLeCSI
///
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
    mpi_mapper_t *mapper = new mpi_mapper_t(machine, rt, *it);
    rt->replace_default_mapper(mapper, *it);
  }
}//mapper registration



}//namespace execution
}//namespace flecsi


#endif //flecsi_execution_mpilegion_maper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

