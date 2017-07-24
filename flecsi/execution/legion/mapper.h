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

clog_register_tag(legion_mapper);

//----------------------------------------------------------------------------//
//! Mapper ID 
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//
enum {
  MPI_MAPPER_ID       = 1,
};

#if 0
//----------------------------------------------------------------------------//
//! mapper tag's IDs
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//
enum {
  // Use the first 8 bits for storing the rhsf index
  MAPPER_FORCE_RANK_MATCH = 0x00001000,
  MAPPER_COMPACTED_STORAGE = 0x00002000,
  MAPPER_SUBRANK_LAUNCH   = 0x00080000,
};
#endif

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The mpi_mapper_t - is a custom mapper that handles mpi-legion
//! interoperability in FLeCSI
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//
class mpi_mapper_t : public Legion::Mapping::DefaultMapper
{
  public:

  //--------------------------------------------------------------------------//
  //! Contructor. Derives from the Legion's Default Mapper
  //!
  //! @param machine Machine type for Legion's Realm
  //! @param _runtime Legion runtime
  //! @param local processor type: currently supports only 
  //!         LOC_PROC and TOC_PROC
  //--------------------------------------------------------------------------//
  mpi_mapper_t(
    Legion::Machine machine,
    Legion::Runtime *_runtime,
    Legion::Processor local
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
    using legion_machine=Legion::Machine;
    using legion_proc=Legion::Processor;

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
      } // end for
    } // end for

    {
    clog_tag_guard(legion_mapper);
    clog(info) <<  "Mapper constuctor: local=" << local << " cpus=" <<
        local_cpus.size() << " gpus=" << local_gpus.size() <<
        " sysmem=" << local_sysmem<<std::endl;
    }
  } // end mpi_mapper_t

  //-------------------------------------------------------------------------//
  //! Destructor
  //-------------------------------------------------------------------------//
  virtual ~mpi_mapper_t(){};

  //-------------------------------------------------------------------------//
  //! Specialization of the slice_task funtion for FLeCSI
  //! The slice_task call is used by the runtime
  //!  to query the mapper about the best way to distribute
  //!  the points in an index space task launch throughout
  //!  the machine.
  //!  To ensure that legion tasks are executed in the same memory space
  //!  as MPI tasks, one need to specify tag = MAPPER_FORCE_RANK_MATCH
  //!  for the index launch int the code.
  //!  By default, slice-task will perform the didtribution the same way 
  //!  it is done in the DefaultMapper. 
  //! 
  //!  @param ctx Legion's context
  //!  @param task Legion's task
  //!  @param input Input information about task distribution between shards
  //!  @param output Output information about task distribution between 
  //!         shards
  //-------------------------------------------------------------------------//
  virtual
  void
  slice_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      const Legion::Mapping::Mapper::SliceTaskInput& input,
      Legion::Mapping::Mapper::SliceTaskOutput& output
             )
  {
    using legion_proc=Legion::Processor;

    context_t & context_ = context_t::instance();

    size_t wait_on_mpi_task_id =
        context_.task_id<__flecsi_internal_task_key(wait_on_mpi_task)>();
    size_t handoff_to_mpi_task_id = 
        context_.task_id<__flecsi_internal_task_key(handoff_to_mpi_task)>();

    // tag-based decisions here
    if(task.task_id == wait_on_mpi_task_id ||
       task.task_id == handoff_to_mpi_task_id ||
      (task.tag & MAPPER_FORCE_RANK_MATCH) != 0) {

      // expect a 1-D index domain
      assert(input.domain.get_dim() == 1);
      LegionRuntime::Arrays::Rect<1> r = input.domain.get_rect<1>();

      // each point in the domain goes to CPUs assigned to that rank
      output.slices.resize(r.volume());
      size_t idx = 0;

      using legion_machine=Legion::Machine;
      using legion_proc=Legion::Processor;

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
      } // end for
      return;
    } // end if MAPPER_FORCE_RANK_MATCH


  	if((task.tag & MAPPER_SUBRANK_LAUNCH) != 0) {
    	// expect a 1-D index domain
    	assert(input.domain.get_dim() == 1);

    	// send the whole domain to our local processor
    	output.slices.resize(1);
    	output.slices[0].domain = input.domain;
    	output.slices[0].proc = task.target_proc;
    	return;
  	} //end if MAPPER_SUBRANK_LAUNCH


    else{
      DefaultMapper::default_slice_task(task, local_cpus, remote_cpus,
                                        input, output, cpu_slices_cache);
    }//end else
  } //end slice_task


  //-------------------------------------------------------------------------//
  //! Specialization of the map_task funtion for FLeCSI
  //! By default, map_task will execute Legions map_task from DefaultMapper.
  //! In the case the launcher has been tagged with the 
  //! "MAPPER_COMPACTED_STORAGE" tag, mapper will create single physical 
  //! instance for exclusive, shared and ghost partitions for each data handle
  //!
  //!  @param ctx Mapper Context
  //!  @param task Legion's task
  //!  @param input Input information about task mapping
  //!  @param output Output information about task mapping
  //-------------------------------------------------------------------------//
  virtual
  void
  map_task(
    const Legion::Mapping::MapperContext ctx,
    const Legion::Task &task,
    const Legion::Mapping::Mapper::MapTaskInput &input,
    Legion::Mapping::Mapper::MapTaskOutput &output)
      {
    DefaultMapper::map_task(ctx, task, input,output);

    Legion::Memory target_mem = 
     DefaultMapper::default_policy_select_target_memory(ctx, task.target_proc);

    if ( (task.tag & MAPPER_COMPACTED_STORAGE) != 0) {
      //check if we get region requirements for "exclusive, shared and ghost"
      //logical regions for each data handle  
 
      //Filling out "layout_constraints" with the defaults
      Legion::LayoutConstraintSet layout_constraints;
      // No specialization
      layout_constraints.add_constraint(Legion::SpecializedConstraint());
      layout_constraints.add_constraint(Legion::OrderingConstraint());
      // Constrained for the target memory kind
      layout_constraints.add_constraint(Legion::MemoryConstraint(
            target_mem.kind()));
      // Have all the field for the instance available
      std::vector<Legion::FieldID> all_fields;
      layout_constraints.add_constraint(Legion::FieldConstraint()); 

      //FIXME:: add colocation_constraints
      Legion::ColocationConstraint colocation_constraints;

      for (size_t indx=0; indx<task.regions.size();indx++){

          Legion::Mapping::PhysicalInstance result;
          std::vector<Legion::LogicalRegion> regions;
          bool created;

          if (task.regions[indx].tag == EXCLUSIVE_LR){

            clog_assert ((task.regions.size()>=(indx+2)),
             "ERROR:: wrong number of regions passed to the task wirth \
               the  tag = MAPPER_COMPACTED_STORAGE");


            regions.push_back(task.regions[indx].region); 
            regions.push_back(task.regions[indx+1].region);
            regions.push_back(task.regions[indx+2].region);      

            if (!runtime->find_or_create_physical_instance(
              ctx, target_mem, layout_constraints,
              regions, result, created, true/*acquire*/, GC_NEVER_PRIORITY)) {
              clog(fatal)<<"ERROR: FLeCSI mapper failed to allocate instance"<<
              std::endl;
            }//end if

            for (size_t j=0; j<3; j++)
             output.chosen_instances[indx+j].push_back(result);            

            indx=indx+2;

          } else {

            regions.push_back(task.regions[indx].region); 
            if (!runtime->find_or_create_physical_instance(
              ctx, target_mem, layout_constraints,
              regions, result, created, true/*acquire*/, GC_NEVER_PRIORITY)) {
              clog(fatal)<<"ERROR: FLeCSI mapper failed to allocate instance"<<
              std::endl;
            }//end if

            output.chosen_instances[indx].push_back(result);
         
          }//end if
      }// end for

#if 0
      //for each data handle
      for (size_t indx=0; indx<task.regions.size()/3;indx++){

        Legion::Mapping::PhysicalInstance result;
        std::vector<Legion::LogicalRegion> regions;

        // we want our mapper to make physical instances that contain space for 
        // all three different logical regions (exclusive, shared and ghost)
        // so one can use that physical instance to satisfy the mapping
        // of all three region requirements. To do so we populate regions
        // vector with all 3 logical regions and pass it to the 
        // find_or_create_physical_instance method.   
        for (size_t j=0; j<3; j++)
          regions.push_back(task.regions[indx*3+j].region);

        bool created;

        if (!runtime->find_or_create_physical_instance(
          ctx, target_mem, layout_constraints,
          regions, result, created, true/*acquire*/, GC_NEVER_PRIORITY)) {
            clog(fatal)<<"ERROR: FLeCSI mapper failed to allocate instance"<<
            std::endl;
        }//end if

        for (size_t j=0; j<3; j++)
          output.chosen_instances[3*indx+j].push_back(result);

    	} // end for
#endif

    }//end if

  }//map_task


 protected:

  //Legion::Mapping::MapperRuntime  mapper_runtime; 
  std::map<Legion::Processor,
           std::map<Realm::Memory::Kind, Realm::Memory> > proc_mem_map;
  Realm::Memory local_sysmem;
  Realm::Machine machine;
};

//--------------------------------------------------------------------------//
//! mapper_registration is used to replace DefaultMapper with mpi_mapper_t in
//! FLeCSI
//!
//! @ingroup legion-execution
//-------------------------------------------------------------------------//
inline
void
mapper_registration(
    Legion::Machine machine,
    Legion::HighLevelRuntime *rt,
    const std::set<Legion::Processor> &local_procs
                    )
{
  for (std::set<Legion::Processor>::const_iterator
           it = local_procs.begin();
       it != local_procs.end(); it++)
  {
    mpi_mapper_t *mapper = new mpi_mapper_t(machine, rt, *it);
    rt->replace_default_mapper(mapper, *it);
  }
} // mapper registration



} // namespace execution
} // namespace flecsi


#endif //flecsi_execution_mpilegion_maper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
